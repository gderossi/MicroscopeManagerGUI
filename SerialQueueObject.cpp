#include "SerialQueueObject.h"

SerialQueueObject::SerialQueueObject(QObject* mainWindow)
{
	connect(this, SIGNAL(serialMessage(std::string)), mainWindow, SLOT(readFromSerialDevice(std::string)));
}

void SerialQueueObject::sendSignal(std::string message)
{
	emit serialMessage(message);
}
