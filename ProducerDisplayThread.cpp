#include "ProducerDisplayThread.h"

ProducerDisplayThread::ProducerDisplayThread(int bufsize, MicroscopeManager* mm, QObject* mainWindow, int* targetFrameInfo):
	ProducerThread(bufsize, mm),
	mainWindow_(mainWindow),
	targetFrameInfo_(targetFrameInfo),
	bufferCount_(0)
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


void ProducerDisplayThread::Display()
{
	QObject::connect(pix_, SIGNAL(pixmapReady(const QPixmap&, bool)), mainWindow_, SLOT(updateDisplayFrame(const QPixmap&, bool)));

	while (active)
	{
		if (bufferCount_ % targetFrameInfo_[0] == targetFrameInfo_[1])
		{
			try
			{
				while (!masked) {}
				std::memcpy(disBuf_, mm_->GetImageBuffer(), mm_->GetImageBufferSize());
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
}

void ProducerDisplayThread::processPixmap()
{
	pixmapProcessed = true;
}