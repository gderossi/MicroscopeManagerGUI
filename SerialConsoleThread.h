#pragma once

#include <MMThread.h>
#include "MicroscopeManager.h"
#include "ui_MicroscopeManagerGUI.h"


class SerialConsoleThread :
    public MMThread
{
public:
    SerialConsoleThread(std::string deviceName, MicroscopeManager* mm, QLabel* consoleOutput);
    void WaitForThread();

private:
    void SerialListen();

    std::string deviceName_;
    MicroscopeManager* mm_;
    QLabel* consoleOutput_;
    std::thread thd_;
};

