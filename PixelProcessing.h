#pragma once

#include <cstdint>

/*
class GetYuv
{
public:
    GetYuv(const void* data, size_t width, size_t height);
    unsigned Y(size_t x, size_t y) const;
    unsigned U(size_t x, size_t y) const;
    unsigned V(size_t x, size_t y) const;
};
*/

class FrameProcessing
{
public:
    template <class IGetYuv>
    void ProcessFrame(const void* frameData, size_t width, size_t height)
    {
        IGetYuv getYuv(frameData, width, height);
        for (size_t y = 0; y < height; ++y) {
            for (size_t x = 0; x < width; ++x) {
                unsigned Y = getYuv.Y(x, y);
                unsigned U = getYuv.U(x, y);
                unsigned V = getYuv.V(x, y);
            }
        }
    }
};

// TODO:
// FrameProcessing frameProcessor;
// frameProcessor.ProcessFrame<GetYuvFromUYVY>(data, w, h);
// frameProcessor.ProcessFrame<GetYuvFromRGB>(data, w, h);
