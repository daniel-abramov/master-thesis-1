#include "mainwidget.h"
#include "libav.h"

#include <QApplication>

#include <exception>
#include <iostream>



struct FrameCallbackHandler {

    bool stopped;

    bool operator()(const libav::AVFrame& videoFrame, int index) {
        std::cout << "videoFrame #" << index << std::endl;
        std::cout << "videoFrame.IsKey() = " << videoFrame.IsKey() << std::endl;
        std::cout << "videoFrame.GetSourceTimestamp() = " << videoFrame.GetSourceTimestamp() << std::endl;
        std::cout << "videoFrame.GetTimestamp() = " << videoFrame.GetTimestamp() << std::endl;
        std::cout << "videoFrame.GetTimeOffset() = " << videoFrame.GetTimeOffset() << std::endl;
        std::cout << "videoFrame.GetTimeBase() = " << videoFrame.GetTimeBase() << std::endl;
        return true;
    }

    bool operator()(const libav::AVSamples& audioSamples, int index) {
        std::cout << "audioSamples #" << index << std::endl;
        std::cout << "audioSamples.GetSize() = " << audioSamples.GetSize() << std::endl;
        std::cout << "audioSamples.GetCapacity() = " << audioSamples.GetCapacity() << std::endl;
        std::cout << "audioSamples.GetSourceTimestamp() = " << audioSamples.GetSourceTimestamp() << std::endl;
        std::cout << "audioSamples.GetTimestamp() = " << audioSamples.GetTimestamp() << std::endl;
        std::cout << "audioSamples.GetTimeOffset() = " << audioSamples.GetTimeOffset() << std::endl;
        std::cout << "audioSamples.GetTimeBase() = " << audioSamples.GetTimeBase() << std::endl;
        return true;
    }

};

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    /* Unit Test
    libav::AVInputFile inputFile("/Users/daniel/Movies/20150909_111119.mp4");
    libav::AVStream fileStream(inputFile);
    FrameCallbackHandler callback;
    fileStream.Decode(callback);
    */

    MainWidget w;
    w.show();

    return a.exec();
}
