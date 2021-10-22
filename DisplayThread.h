#pragma once

#include <MMThread.h>
#include "MicroscopeManager.h"
#include "ui_MicroscopeManagerGUI.h"

class DisplayThread :
    public MMThread
{
public:
    DisplayThread(unsigned long long bufferCount, MicroscopeManager* mm, QLabel* displayFrame, int* targetFrameInfo);
    ~DisplayThread();
    void WaitForThread();

private:
    void Display();

    unsigned long long bufferCount_;
    std::thread disThd_;
    MicroscopeManager* mm_;
    unsigned char* buf_;
    QImage img_;
    QLabel* displayFrame_;
    int* targetFrameInfo_;
    unsigned long long width;
    unsigned long long height;
};

