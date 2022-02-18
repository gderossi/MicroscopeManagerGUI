#include "DisplayThread.h"
#include <algorithm>

DisplayThread::DisplayThread(unsigned long long bufferCount, MicroscopeManager* mm, QObject* mainWindow, int* targetFrameInfo) :
	bufferCount_(bufferCount),
	mm_(mm),
	mainWindow_(mainWindow),
	targetFrameInfo_(targetFrameInfo)
{
	width = mm->GetCameraIntParameter(STREAM_MODULE, "Width");
	height = mm->GetCameraIntParameter(STREAM_MODULE, "Height");
	buf_ = new unsigned char[width * height];

	pix_ = new PixmapReadyObject();

	disThd_ = std::thread(&DisplayThread::Display, this);
}

DisplayThread::~DisplayThread()
{
	delete[] buf_;
	delete pix_;
}

void DisplayThread::Display()
{
	QObject::connect(pix_, SIGNAL(pixmapReady(const QPixmap&, bool)), mainWindow_, SLOT(updateDisplayFrame(const QPixmap&, bool)));

	while (active)
	{
		mm_->GetImage();
		mm_->ApplyCameraMask();

		if (bufferCount_ % targetFrameInfo_[0] == targetFrameInfo_[1])
		{
			try
			{
				std::memcpy(buf_, mm_->GetImageBuffer(), std::min(mm_->GetImageBufferSize(), width*height));
				QImage img = QImage(buf_, width, height, width, QImage::Format_Grayscale8);
				const QPixmap pixmap = QPixmap::fromImage(img);
				pixmapProcessed = false;
				pix_->sendSignal(pixmap, false);
			}
			catch (...)
			{
				active = false;
			}
		}

		if (--bufferCount_ == 0)
		{
			active = false;
		}
	}
}

void DisplayThread::WaitForThread()
{
	disThd_.join();
}
