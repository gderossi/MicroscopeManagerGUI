#include "SerialDeviceThread.h"

SerialDeviceThread::SerialDeviceThread(SerialDevice* device, SerialQueueObject* serialQueue) :
	device_(device),
	serialQueue_(serialQueue)
{
	thd_ = std::thread(&SerialDeviceThread::Listen, this);
}

void SerialDeviceThread::Listen()
{
	while (active)
	{
		if (device_->Available())
		{
			serialQueue_->qMutex.lock();
			std::string data = device_->ReadData(128);
			serialQueue_->sendSignal(data);
			serialQueue_->qMutex.unlock();
		}
	}
}

void SerialDeviceThread::WaitForThread()
{
	thd_.join();
}