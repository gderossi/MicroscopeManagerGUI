#pragma once
#include "ProducerThread.h"
#include "qwidget.h"
#include "PixmapReadyObject.h"

class ProducerDisplayThread :
    public ProducerThread
{
public:
    ProducerDisplayThread(int bufsize, MicroscopeManager* mm, QObject* mainWindow, int* targetFrameInfo);
    ~ProducerDisplayThread();
    void StartThreads();
    void WaitForThread();
    void processPixmap();

private:
    void CheckFrameInfo();
    void Display();

    std::thread disThd_;
    unsigned char* disBuf_;
    QObject* mainWindow_;
    int* targetFrameInfo_;
    unsigned long long width;
    unsigned long long height;
    float framerate;
    std::atomic_bool pixmapProcessed;
    PixmapReadyObject* pix_;
    unsigned long long bufferCount_;
    bool endDisplay_;
    int frameCount_;
    int currentFrame_;
    int framesToDrop_;
    std::thread frameThd_;
};

