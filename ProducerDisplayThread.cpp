#include "ProducerDisplayThread.h"

ProducerDisplayThread::ProducerDisplayThread(int bufsize, MicroscopeManager* mm, QObject* mainWindow, int* targetFrameInfo):
	ProducerThread(bufsize, mm),
	mainWindow_(mainWindow),
	targetFrameInfo_(targetFrameInfo),
	bufferCount_(0),
	endDisplay_(false)
{
	width = mm->GetCameraIntParameter(STREAM_MODULE, "Width");
	height = mm->GetCameraIntParameter(STREAM_MODULE, "Height");
	disBuf_ = new unsigned char[width * height];
	pix_ = new PixmapReadyObject();
}

ProducerDisplayThread::~ProducerDisplayThread()
{}

void ProducerDisplayThread::StartThreads()
{
	StartThread();
	disThd_ = std::thread(&ProducerDisplayThread::Display, this);
}

void ProducerDisplayThread::WaitForThread()
{
	{
		std::unique_lock<std::mutex> ul(pMutex);
		endDisplay_ = true;
	}
	pCV.notify_all();

	disThd_.join();
	ProducerThread::WaitForThread();
}

void ProducerDisplayThread::Display()
{
	QObject::connect(pix_, SIGNAL(pixmapReady(const QPixmap&, bool)), mainWindow_, SLOT(updateDisplayFrame(const QPixmap&, bool)));

	int targetFrame;

	while (active)
	{
		if (targetFrameInfo_[1] < bufReadys.size())
		{
			targetFrame = targetFrameInfo_[1];
		}
		else
		{
			targetFrame = 0;
		}
		{
			std::unique_lock<std::mutex> ul(pMutex);
			if (!bufReadys[targetFrame])
			{
				pCV.wait(ul, [this, targetFrame]() {return bufReadys[targetFrame] || endDisplay_; });
			}
		}

		if (!active)
		{
			return;
		}
		
		try
		{
			std::memcpy(disBuf_, imgBuffers[targetFrame], bufSize);
			QImage img = QImage(disBuf_, width, height, width, QImage::Format_Grayscale8);
			const QPixmap pixmap = QPixmap::fromImage(img);
			pixmapProcessed = false;
			pix_->sendSignal(pixmap, true);
			//while(!pixmapProcessed){} //Stall until successful display
		}
		catch (...)
		{
			active = false;
		}
	}
}

void ProducerDisplayThread::processPixmap()
{
	pixmapProcessed = true;
}