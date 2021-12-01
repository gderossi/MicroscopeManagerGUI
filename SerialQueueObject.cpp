#include "SerialQueueObject.h"

void SerialQueueObject::sendSignal(std::string message)
{
	emit serialMessage(message);
}
