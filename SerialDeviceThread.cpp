#include "SerialDeviceThread.h"

SerialDeviceThread::SerialDeviceThread(SerialDevice* device, SerialQueueObject* serialQueue, QObject* mainWindow) :
	device_(device),
	serialQueue_(serialQueue),
	mainWindow_(mainWindow)
{
	thd_ = std::thread(&SerialDeviceThread::Listen, this);
}

void SerialDeviceThread::Listen()
{
	QObject::connect(serialQueue_, SIGNAL(serialMessage(std::string)), mainWindow_, SLOT(readFromSerialDevice(std::string)));

	while (active)
	{
		if (device_->Available())
		{
			const std::lock_guard<std::mutex> lock(serialQueue_->qMutex);
			serialQueue_->sendSignal(device_->ReadData(128));
		}
	}
}

void SerialDeviceThread::WaitForThread()
{
	thd_.join();
}