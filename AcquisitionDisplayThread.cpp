#include "AcquisitionDisplayThread.h"
#include "MicroscopeManagerGUI.h"

AcquisitionDisplayThread::AcquisitionDisplayThread(unsigned long long bufferCount, MicroscopeManager* mm, QObject* mainWindow, int* targetFrameInfo) :
	bufferCount_(bufferCount),
	mm_(mm),
	mainWindow_(mainWindow),
	targetFrameInfo_(targetFrameInfo)
{
	width = mm->GetCameraIntParameter(STREAM_MODULE, "Width");
	height = mm->GetCameraIntParameter(STREAM_MODULE, "Height");
	buf_ = new unsigned char[width * height];

	pix_ = new PixmapReadyObject();

	acqThd_ = std::thread(&AcquisitionDisplayThread::Acquire, this);
	disThd_ = std::thread(&AcquisitionDisplayThread::Display, this);
}

AcquisitionDisplayThread::~AcquisitionDisplayThread()
{
	delete[] buf_;
	delete pix_;
}

void AcquisitionDisplayThread::Acquire()
{
	while (active)
	{
		try
		{
			mm_->GetImage();
			mm_->WriteFile(mm_->GetImageBuffer(), mm_->GetImageBufferSize());
		}
		catch (...)
		{
			active = false;
		}

		if (--bufferCount_ == 0)
		{
			active = false;
		}
	}
}

void AcquisitionDisplayThread::Display()
{
	QObject::connect(pix_, SIGNAL(pixmapReady(const QPixmap&, bool)), mainWindow_, SLOT(updateDisplayFrame(const QPixmap&, bool)));

	while (active)
	{
		if (bufferCount_ % targetFrameInfo_[0] == targetFrameInfo_[1])
		{
			try
			{
				std::memcpy(buf_, mm_->GetImageBuffer(), mm_->GetImageBufferSize());
				QImage img = QImage(buf_, width, height, width, QImage::Format_Grayscale8);
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

void AcquisitionDisplayThread::WaitForThread()
{
	acqThd_.join();
	disThd_.join();
}

void AcquisitionDisplayThread::processPixmap()
{
	pixmapProcessed = true;
}
