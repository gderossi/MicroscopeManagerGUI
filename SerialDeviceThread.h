#pragma once
#include <MMThread.h>
#include "serial/serial.h"
#include <SerialDevice.h>
#include <SerialQueueObject.h>

class SerialDeviceThread :
    public MMThread
{
public:
    SerialDeviceThread(SerialDevice* device, SerialQueueObject* serialQueue);
    void WaitForThread();
    
private:
    void Listen();

    SerialDevice* device_;
    SerialQueueObject* serialQueue_;
    std::thread thd_;
};

