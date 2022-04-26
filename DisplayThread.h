#pragma once

#include "MMThread.h"
#include "MicroscopeManager.h"
#include "ui_MicroscopeManagerGUI.h"
#include "PixmapReadyObject.h"

class DisplayThread :
    public MMThread
{
public:
    DisplayThread(unsigned long long bufferCount, MicroscopeManager* mm, QObject* mainWindow, int* targetFrameInfo);
    ~DisplayThread();
    void WaitForThread();

private:
    void Display();
    void CheckFrameInfo();

    unsigned long long bufferCount_;
    std::thread disThd_;
    std::thread frameThd_;
    MicroscopeManager* mm_;
    unsigned char* buf_;
    int* targetFrameInfo_;
    QObject* mainWindow_;
    unsigned long long width;
    unsigned long long height;
    std::atomic_bool pixmapProcessed;
    PixmapReadyObject* pix_;
    int frameCount_;
    int currentFrame_;
};

