#pragma once

#include <MMThread.h>
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

    unsigned long long bufferCount_;
    std::thread disThd_;
    MicroscopeManager* mm_;
    unsigned char* buf_;
    QObject* mainWindow_;
    int* targetFrameInfo_;
    unsigned long long width;
    unsigned long long height;
    std::atomic_bool pixmapProcessed;
    PixmapReadyObject* pix_;
};

