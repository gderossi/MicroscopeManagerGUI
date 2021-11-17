#pragma once

#include "MMThread.h"
#include "MicroscopeManager.h"
#include "qwidget.h"
#include "PixmapReadyObject.h"

class AcquisitionDisplayThread :
	public MMThread
{
public:
	AcquisitionDisplayThread(unsigned long long bufferCount, MicroscopeManager* mm, QObject* mainWindow, int* targetFrameInfo);
	~AcquisitionDisplayThread();
	void WaitForThread();
	void processPixmap();

private:
	void Acquire();
	void Display();

	unsigned long long bufferCount_;
	std::thread acqThd_;
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

