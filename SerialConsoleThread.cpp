#include "SerialConsoleThread.h"

SerialConsoleThread::SerialConsoleThread(std::string deviceName, MicroscopeManager* mm, QLabel* consoleOutput, unsigned int maxSize) :
	deviceName_(deviceName),
	mm_(mm),
	consoleOutput_(consoleOutput),
	maxSize_(maxSize),
	thd_(&SerialConsoleThread::SerialListen, this)
{}

void SerialConsoleThread::WaitForThread()
{
	thd_.join();
}

void SerialConsoleThread::SerialListen()
{
	std::string output;

	while (active)
	{
		if ((output = mm_->SerialRead(deviceName_, 128)) != "")
		{
			output += '\n';
			output = consoleOutput_->text().toUtf8().constData() + output;
			consoleOutput_->setText(output.c_str());
		}
	}
}
