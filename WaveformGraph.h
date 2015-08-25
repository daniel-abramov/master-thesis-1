#include "IPixelProcessor.h"

class WaveformGraph : public IPixelProcessor
{
private:
    virtual void ProcessUYVY(void* data, size_t line_size, size_t frame_size) override
    {
    }

    virtual void ProcessRGB(void* data, size_t line_size, size_t frame_size) override
    {
    }
};
