#pragma once

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <exception>
#include <iostream>
#include <memory>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/avutil.h>
}

#undef M_LOG2_10

// TODO: Move to the other place
class NoCopy
{
public:
    NoCopy() = default;
    NoCopy(const NoCopy&) = delete;
    NoCopy& operator=(const NoCopy&) = delete;
};

namespace libav {

class AVError : public std::exception
{
public:
    AVError(const char* name, int err = 0)
    {
        std::cerr << name << ": ";
        if (err) {
            if (err == -1) {
                std::cerr << "unspecified error (-1) ";
            } else {
                char buf[1024];
                if (0 == ::av_strerror(err, buf, sizeof(buf)))
                    std::cerr << buf << " ";
                else
                    std::cerr << "unknown error ";
            }
        }
    }

    template< typename T >
    AVError& operator<<(const T& arg)
    {
        std::cerr << arg;
        return *this;
    }
};


class AVInit : NoCopy
{
protected:
    AVInit()
    {
        static bool initialized = false;
        if (! initialized) {
            ::av_register_all();
            initialized = true;
        }
    }
};


// Custom `in memory` access to data
class IAVDataSource
{
public:
    ::AVIOContext* AllocContext()
    {
        size_t bufferSize = PreferredSize();
        unsigned char* buffer = static_cast<unsigned char*>(::av_malloc(bufferSize));
        if (! buffer)
            throw AVError("av_malloc");
        ::AVIOContext* ret = ::avio_alloc_context(buffer, bufferSize, 0, this, DoRead, nullptr, DoSeek);
        if (! ret)
            throw AVError("avio_alloc_context");
        if (! Seekable())
            ret->seekable = 0;
        return ret;
    }

    bool GetOkToRead() const { return OkToRead(); }
    uint64_t GetSize() const { return Size(); }
    bool GetSeekable() const { return Seekable(); }

    virtual size_t Read(void* buf, size_t size) = 0;
    virtual int64_t Seek(int64_t /*offset*/, int /*whence*/) { return -1; }
    virtual size_t PreferredSize() const { return 1<<20; }

protected:
    virtual ~IAVDataSource() { }
    virtual uint64_t Size() const { return 0; }
    virtual bool Seekable() const { return false; }
    virtual bool OkToRead() const { return true; }

private:
    static IAVDataSource& This(void* ptr)
    {
        return *reinterpret_cast<IAVDataSource*>(ptr);
    }

    static int DoRead(void* opaque, uint8_t* buf, int size)
    {
        return This(opaque).Read(buf, size);
    }

    static int64_t DoSeek(void* opaque, int64_t offset, int whence)
    {
        if (AVSEEK_SIZE == whence)
            return This(opaque).Size();
        if (! This(opaque).Seekable())
            return -1;
        return This(opaque).Seek(offset, whence);
    }
};


class AVInputFile;


struct AVCodec
{
    friend class AVInputFile;
    friend class AVStream;

    const AVInputFile&       input;
    ::AVCodecContext* const  context;
    const ::AVRational       timeBase;
    const unsigned           index;

private:
    AVCodec(const AVInputFile& inp, ::AVCodecContext* ctx, ::AVRational tb, unsigned idx)
        : input(inp)
        , context(ctx)
        , timeBase(tb)
        , index(idx)
    { }

    AVCodec(const AVCodec& other)
        : AVCodec(other.input, other.context, other.timeBase, other.index)
    { }

    AVCodec& operator=(const AVCodec&) = delete;
};


class AVInputFile : public AVInit
{
    friend class AVPacket;

public:
    AVInputFile(const char* uri)
        : m_formatCtx(Construct(uri))
        , m_source(nullptr)
    {
        ::avformat_find_stream_info(m_formatCtx, nullptr); // ignore errors
    }

    AVInputFile(IAVDataSource& source, const char* name)
        : m_formatCtx(Construct(source, name))
        , m_source(&source)
    {
        ::avformat_find_stream_info(m_formatCtx, nullptr); // ignore errors
    }

    virtual ~AVInputFile()
    {
        ::avformat_close_input(const_cast<::AVFormatContext**>(&m_formatCtx));
    }

    const char* GetFileName() const { return m_formatCtx->filename; }
    uint64_t GetDuration() const { return (uint64_t)m_formatCtx->duration * 1000000 / AV_TIME_BASE; } // microseconds
    uint64_t GetFileSize() const { return 0; /* TODO: m_formatCtx->filesize; */ } // bytes
    unsigned GetBitRate() const { return m_formatCtx->bit_rate; }
    const char* GetContainerName() const { return m_formatCtx->iformat->name; }

    bool Stopped() const
    {
        return m_source ? !m_source->GetOkToRead() : false;
    }

    AVCodec FindStream(int type) const
    {
        for (unsigned i = 0; i < m_formatCtx->nb_streams; ++i) {
            if (m_formatCtx->streams[i]->codec->codec_type == type)
                return AVCodec(*this, m_formatCtx->streams[i]->codec, m_formatCtx->streams[i]->time_base, i);
        }
        throw AVError("FindStream") << "no stream of type " << type;
    }

private:
    static inline ::AVFormatContext* Construct(const char* uri)
    {
        ::AVFormatContext* ret = nullptr;
        int err = ::avformat_open_input(&ret, uri, nullptr, nullptr);
        if (err)
            throw AVError("av_open_input_file", err);
        return ret;
    }

    static inline ::AVFormatContext* Construct(IAVDataSource& source, const char* name)
    {
        ::AVFormatContext* ret = ::avformat_alloc_context();
        ret->flags |= AVFMT_FLAG_CUSTOM_IO;
        ret->pb = source.AllocContext();
        int err = ::avformat_open_input(&ret, name, nullptr, nullptr);
        if (err)
            throw AVError("av_open_input_file", err);
        return ret;
    }

private:
    ::AVFormatContext* const    m_formatCtx;
    const IAVDataSource* const  m_source;
};


class AVPacket : NoCopy
{
public:
    AVPacket()
    {
        m_rawPacket.data = nullptr;
        m_rawPacket.size = 0;
        m_packet = m_rawPacket;
        m_complete = false;
    }

    virtual ~AVPacket()
    {
        Free();
    }

    ::AVPacket* GetPacket() { return &m_packet; }

    bool Read(const AVInputFile& in, bool& stopped)
    {
        stopped = false;
        m_complete = false;
        if (in.Stopped()) {
            stopped = true;
            return false;
        }

        Free();
        if (::av_read_frame(in.m_formatCtx, &m_rawPacket) < 0)
            return false;

        m_packet = m_rawPacket;
        m_complete = true;
        return true;
    }

    void Consume(size_t size)
    {
        if (m_packet.data) {
            size = (size > (unsigned)m_packet.size) ? (unsigned)m_packet.size : size;
            m_packet.size -= size;
            m_packet.data += size;
        }
    }

    size_t Size() const
    {
        return m_packet.size;
    }

    unsigned Index() const
    {
        return (unsigned)m_packet.stream_index;
    }

    bool Empty() const
    {
        return (! m_complete) || (m_packet.size == 0);
    }

    void Free()
    {
        if (m_rawPacket.data)
            ::av_free_packet(&m_rawPacket);
        m_rawPacket.data = nullptr;
        m_rawPacket.size = 0;
    }

private:
    ::AVPacket  m_packet;
    ::AVPacket  m_rawPacket;
    bool        m_complete;
};


class AVFrameBase : NoCopy
{
public:
    AVFrameBase()
        : m_timeBase(1.0)
        , m_timeOffset(0)
    { }

    double GetTimeBase() const { return m_timeBase; }
    void SetTimeBase(double timeBase) { m_timeBase = timeBase; }

    uint64_t GetTimeOffset() const { return m_timeOffset; }
    void SetTimeOffset(uint64_t timeOffset) { m_timeOffset = timeOffset; }

protected:
    uint64_t MakeTimestamp(uint64_t src) const
    {
        const uint64_t timestamp = static_cast<uint64_t>(src * m_timeBase * 1000.0);
        return std::max(timestamp, m_timeOffset) - m_timeOffset;
    }

private:
    double    m_timeBase;
    uint64_t  m_timeOffset;
};


class AVFrame : public AVFrameBase
{
public:
    AVFrame()
    {
        ::av_frame_unref(&m_frame);
    }

    const ::AVFrame* GetRaw() const { return &m_frame; }
    ::AVFrame* GetRaw() { return &m_frame; }
    const ::AVPicture* GetPicture() const { return reinterpret_cast<const ::AVPicture*>(&m_frame); }
    ::AVPicture* GetPicture() { return reinterpret_cast< ::AVPicture*>(&m_frame); }

    bool IsKey() const { return m_frame.key_frame; }
    uint32_t GetWidth() const { return m_frame.width; }
    uint32_t GetHeight() const { return m_frame.height; }
    uint64_t GetSourceTimestamp() const { return m_frame.best_effort_timestamp; }
    void SetSourceTimestamp(uint64_t timestamp) { m_frame.best_effort_timestamp = timestamp; }
    uint64_t GetTimestamp() const { return MakeTimestamp(GetSourceTimestamp()); }

    const uint8_t* GetPlane(unsigned idx) const { assert(idx < 4); return m_frame.data[idx]; }
    unsigned GetLineSize(unsigned idx) const { assert(idx < 4); return m_frame.linesize[idx]; }

private:
    ::AVFrame m_frame;
};


struct AVSampleFormat
{
    ::AVSampleFormat  format;
    unsigned          channels;
    unsigned          sampleRate;

    AVSampleFormat(::AVSampleFormat format, unsigned channels, unsigned sampleRate)
        : format(format)
        , channels(channels)
        , sampleRate(sampleRate)
    { }
};


class AVSamples : public AVFrameBase
{
public:
    AVSamples()
        : m_capacity(192000) // 1 second of 48khz 32 bit audio
        , m_samples(static_cast<int16_t*>(::av_malloc(m_capacity)))
        , m_size(0)
        , m_timestamp(0)
        , m_codecCtx(nullptr)
    { }

    ~AVSamples()
    {
        ::av_free(m_samples);
    }

    int16_t* GetRaw() { return m_samples; }
    const int16_t* GetRaw() const { return m_samples; }

    size_t GetSize() const { return m_size; }
    void SetSize(size_t size) { m_size = size; }

    size_t GetCapacity() const { return m_capacity; }

    uint64_t GetSourceTimestamp() const { return m_timestamp; }
    void SetSourceTimestamp(uint64_t timestamp) { m_timestamp = timestamp; }
    uint64_t GetTimestamp() const { return MakeTimestamp(GetSourceTimestamp()); }

    AVSampleFormat GetFormat() const
    {
        return AVSampleFormat(m_codecCtx->sample_fmt, m_codecCtx->channels, m_codecCtx->sample_rate);
    }

    ::AVCodecContext* GetCodecContext() const { return m_codecCtx; }
    void SetCodecContext(::AVCodecContext* ctx) { m_codecCtx = ctx; }

private:
    const size_t  m_capacity;
    int16_t*      m_samples;
    size_t        m_size;
    uint64_t      m_timestamp;

    ::AVCodecContext* m_codecCtx;
};


class AVCodecBase : NoCopy
{
public:
    virtual ~AVCodecBase()
    {
        ::avcodec_close(m_codecCtx.get());
    }

    const char* GetCodecName() const
    {
        return m_codec ? m_codec->name : "";
    }

protected:
    static inline ::AVCodec* Construct(::AVCodecContext* ctx)
    {
        ::AVCodec* ret = ::avcodec_find_decoder(ctx->codec_id);
        if (! ret)
            throw AVError("avcodec_find_decoder");
        return ret;
    }

    AVCodecBase(::AVCodecContext* ctx)
        : m_codec(Construct(ctx))
        , m_codecCtx(ctx, ::av_free)
    {
    }

    AVCodecBase(::AVCodec* cdc)
        : m_codec(cdc)
        , m_codecCtx(::avcodec_alloc_context3(m_codec), ::av_free)
    {
        if (! m_codecCtx)
            throw AVError("avcodec_alloc_context3");
    }

    virtual void PrepareContext()
    {
        if (m_codec->capabilities & CODEC_CAP_TRUNCATED)
           m_codecCtx->flags |= CODEC_FLAG_TRUNCATED;
    }

    void Init()
    {
        PrepareContext();
        int err = ::avcodec_open2(m_codecCtx.get(), m_codec, nullptr);
        if (err)
            throw AVError("avcodec_open2", err);
    }

protected:
    ::AVCodec* const m_codec;

    typedef decltype(::av_free) *AVFreeFunc;
    std::unique_ptr<AVCodecContext, AVFreeFunc> const m_codecCtx;
};


class AVVideoEngine : NoCopy
{
public:
    typedef AVFrame TFrameType;

    int GetStreamType() const { return AVMEDIA_TYPE_VIDEO; }
    const char* GetName() const { return "avcodec_decode_video2"; }

    int Decode(::AVCodecContext* ctx, AVFrame& frame, AVPacket& packet, int& finished)
    {
        return ::avcodec_decode_video2(ctx, frame.GetRaw(), &finished, packet.GetPacket());
    }
};


class AVAudioEngine : NoCopy
{
public:
    typedef AVSamples TFrameType;

    int GetStreamType() const { return AVMEDIA_TYPE_AUDIO; }
    const char* GetName() const { return "avcodec_decode_audio3"; }

    int Decode(::AVCodecContext* ctx, AVSamples& samples, AVPacket& packet, int& finished)
    {
        finished = samples.GetCapacity();
        int ret = ::avcodec_decode_audio3(ctx, samples.GetRaw(), &finished, packet.GetPacket());
        samples.SetSize(finished);
        samples.SetSourceTimestamp(static_cast<uint64_t>(packet.GetPacket()->pts));
        samples.SetCodecContext(ctx);
        return ret;
    }
};


template< typename TEngine >
class AVEngineDecoder : AVInit, public AVCodecBase
{
public:
    AVEngineDecoder(const AVCodec& c, TEngine& engine)
        : AVCodecBase(c.context)
        , m_engine(engine)
        , m_timeBase(::av_q2d(c.timeBase))
    {
        Init();
    }

    bool DecodeFrame(typename TEngine::TFrameType& frame, AVPacket& packet, bool& failed)
    {
        frame.SetTimeBase(m_timeBase);

        int finished = 0;
        while (! packet.Empty() && ! finished) {
            int s = m_engine.Decode(m_codecCtx.get(), frame, packet, finished);
            if (s < 0) {
                if (failed)
                    throw AVError(m_engine.GetName());
                failed = true;
            }
            packet.Consume(s > 0 ? s : packet.Size());
        }
        return finished;
    }

private:
    TEngine&      m_engine;
    const double  m_timeBase;
};


class AVStream : NoCopy
{
    template< typename TEngine >
    struct Worker {
        typename TEngine::TFrameType  frame;
        TEngine                       engine;
        AVCodec                       codec;
        AVEngineDecoder< TEngine >    decoder;
        uint64_t                      timeOffset;
        unsigned                      index;
        bool                          failed;
        bool                          cont;

        Worker(const AVInputFile& inp)
            : codec(inp.FindStream(engine.GetStreamType()))
            , decoder(codec, engine)
            , timeOffset(0)
            , index(0)
            , failed(false)
            , cont(true)
        { }

        template< typename TCallback >
        bool Decode(AVPacket& packet, TCallback& callback)
        {
            if (packet.Index() == codec.index) {
                while (cont && decoder.DecodeFrame(frame, packet, failed)) {
                    frame.SetTimeOffset(timeOffset);
                    if (! index) {
                        timeOffset = frame.GetTimestamp();
                        frame.SetTimeOffset(timeOffset);
                    }
                    cont &= callback(frame, ++index);
                }
            }
            return cont;
        }
    };

public:
    AVStream(const AVInputFile& inp)
        : m_input(inp)
        , m_videoWorker(inp)
        , m_audioWorker(inp)
    {
    }

    const char* GetCodecName() const
    {
        return m_videoWorker.decoder.GetCodecName();
    }

    template< typename TCallback >
    bool Decode(TCallback& callback, bool decodeSinglePacket = false) // by default decode all packets (whole file)
    {
        while (m_packet.Empty()) {
            if (! m_packet.Read(m_input, callback.stopped))
                return false;

            bool video = m_videoWorker.Decode(m_packet, callback);
            bool audio = m_audioWorker.Decode(m_packet, callback);
            if (! video && ! audio)
                return true;
            m_packet.Consume(m_packet.Size());

            if (decodeSinglePacket)
                return true;
        }
        return true;
    }

private:
    const AVInputFile& m_input;

    AVPacket m_packet;

    Worker< AVVideoEngine > m_videoWorker;
    Worker< AVAudioEngine > m_audioWorker;
};


struct AVImageFormat
{
    unsigned       width;
    unsigned       height;
    ::PixelFormat  format;

    AVImageFormat(unsigned width, unsigned height, ::PixelFormat format)
        : width(width)
        , height(height)
        , format(format)
    { }
};


class AVImageConvert : public NoCopy
{
public:
    AVImageConvert(const AVImageFormat& src, const AVImageFormat& dst)
        : m_swsCtx(Construct(src, dst))
    {
    }

    virtual ~AVImageConvert()
    {
        if (m_swsCtx)
            ::sws_freeContext(m_swsCtx);
    }

    void Convert(AVFrame& dst, const AVFrame& src)
    {
        const ::AVFrame& s = *src.GetRaw();
        ::AVFrame& d = *dst.GetRaw();
        int err = ::sws_scale(m_swsCtx, s.data, s.linesize, 0, s.height, d.data, d.linesize);
        if (err < 0)
            throw AVError("sws_scale", err);
    }

private:
    static inline struct ::SwsContext* Construct(const AVImageFormat& src, const AVImageFormat& dst)
    {
        struct ::SwsContext* ret = ::sws_getCachedContext(nullptr,
                                                          src.width, src.height, src.format,
                                                          dst.width, dst.height, dst.format,
                                                          SWS_SINC,
                                                          nullptr, nullptr, nullptr);
        if (! ret)
            throw AVError("sws_getCachedContext") << "(" << src.width << "," << src.height << "," << (int)src.format
                                                  << ") -> (" << dst.width << "," << dst.height << "," << (int)dst.format << ")";
        return ret;
    }

private:
    struct ::SwsContext* const m_swsCtx;
};


class AVSampleConvert : public NoCopy
{
    static const int FILTER_LENGTH = 16;
    static const int LOG2_PHASE_COUNT = 10;
    static const bool LINEAR = false;

public:
    AVSampleConvert(const AVSampleFormat& src, const AVSampleFormat& dst)
        : m_reSampleCtx(Construct(src, dst))
        , m_src(src)
        , m_dst(dst)
    { }

    virtual ~AVSampleConvert()
    {
        if (m_reSampleCtx)
            ::audio_resample_close(m_reSampleCtx);
    }

    void Convert(AVSamples& dst, const AVSamples& src)
    {
        short* const input = const_cast<short*>(src.GetRaw());
        short* const output = static_cast<short*>(dst.GetRaw());
        const int nb_samples_src = src.GetSize() / (::av_get_bytes_per_sample(m_src.format) * m_src.channels);
        const int nb_samples_dst = ::audio_resample(m_reSampleCtx, output, input, nb_samples_src);
        if (nb_samples_dst < 0)
            throw AVError("audio_resample");
        dst.SetSize(nb_samples_dst * ::av_get_bytes_per_sample(m_dst.format) * m_dst.channels);
    }

private:
    static inline struct ::ReSampleContext* Construct(const AVSampleFormat& src, const AVSampleFormat& dst)
    {
        struct ::ReSampleContext* ret = ::av_audio_resample_init(dst.channels, src.channels,
                                                               dst.sampleRate, src.sampleRate,
                                                               dst.format, src.format,
                                                               FILTER_LENGTH, LOG2_PHASE_COUNT,
                                                               LINEAR ? 1 : 0, 1.0);
        if (! ret)
            throw AVError("av_audio_resample_init");

        return ret;
    }

private:
    struct ::ReSampleContext* const m_reSampleCtx;
    AVSampleFormat m_src;
    AVSampleFormat m_dst;
};


class AVTempFrame : public AVFrame
{
public:
    AVTempFrame(const AVImageFormat& fmt)
    {
        AllocFrame(fmt);
    }

    AVTempFrame(const AVImageFormat& fmt, const AVFrame& src)
    {
        ConvertFrame(fmt, src);
    }

    AVTempFrame(unsigned width, unsigned height, ::PixelFormat format, const AVFrame& src)
    {
        AVImageFormat fmt(width, height, format);
        ConvertFrame(fmt, src);
    }

    virtual ~AVTempFrame()
    {
        ::avpicture_free(GetPicture());
    }

private:
    void AllocFrame(const AVImageFormat& fmt)
    {
        ::avpicture_alloc(GetPicture(), fmt.format, fmt.width, fmt.height);
    }

    void ConvertFrame(const AVImageFormat& fmt, const AVFrame& src)
    {
        AllocFrame(fmt);
        const ::AVFrame& rf = *src.GetRaw();
        AVImageFormat srcf(rf.width, rf.height, static_cast<enum ::PixelFormat>(rf.format));
        AVImageConvert conv(srcf, fmt);
        conv.Convert(*this, src);
        SetTimeBase(src.GetTimeBase());
        SetTimeOffset(src.GetTimeOffset());
        SetSourceTimestamp(src.GetSourceTimestamp());
    }
};


class AVTempSamples : public AVSamples
{
public:
    AVTempSamples(const AVSampleFormat& fmt, const AVSamples& src)
    {
        ConvertSamples(fmt, src);
    }

    AVTempSamples(unsigned channels, unsigned rate, ::AVSampleFormat format, const AVSamples& src)
    {
        AVSampleFormat fmt(format, channels, rate);
        ConvertSamples(fmt, src);
    }

private:
    void ConvertSamples(const AVSampleFormat& fmt, const AVSamples& src)
    {
        ::AVCodecContext* ctx = src.GetCodecContext();
        if (! ctx)
            throw AVError("ctx");
        AVSampleFormat sfmt(ctx->sample_fmt, ctx->channels, ctx->sample_rate);
        AVSampleConvert convert(sfmt, fmt);
        convert.Convert(*this, src);
        SetTimeBase(src.GetTimeBase());
        SetTimeOffset(src.GetTimeOffset());
        SetSourceTimestamp(src.GetSourceTimestamp());
        SetCodecContext(src.GetCodecContext());
    }
};


class AVThumbnailEncoder : AVInit, public AVCodecBase
{
public:
    AVThumbnailEncoder(const char* codec, unsigned width, unsigned height)
        : AVCodecBase(Construct(codec))
        , m_dstf(width, height, m_codec->pix_fmts[0])
    {
        Init();
    }

    size_t Encode(void *buf, size_t size, const AVFrame& frame)
    {
        AVTempFrame df(m_dstf, frame);
        int s = ::avcodec_encode_video(m_codecCtx.get(), (uint8_t*)buf, size, df.GetRaw());
        if (s < 0)
            throw AVError("avcodec_encode_video", s);
        return s;
    }

    uint32_t GetWidth()  const { return m_dstf.width; }
    uint32_t GetHeight() const { return m_dstf.height; }

protected:
    virtual void PrepareContext()
    {
        m_codecCtx->pix_fmt = m_dstf.format;
        m_codecCtx->width = m_dstf.width;
        m_codecCtx->height = m_dstf.height;
    }

private:
    static inline ::AVCodec* Construct(const char* codec)
    {
        ::AVCodec* ret = ::avcodec_find_encoder_by_name(codec);
        if  (! ret)
            throw AVError("avcodec_find_encoder_by_name") << "No such codec: " << codec;
        return ret;
    }

private:
    AVImageFormat m_dstf;
};

} // namespace libav
