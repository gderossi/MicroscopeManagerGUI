#include "DisplayThread.h"
#include <algorithm>

#define MAX_FRAMES_PER_SECOND 30

DisplayThread::DisplayThread(unsigned long long bufferCount, MicroscopeManager* mm, QObject* mainWindow, int* targetFrameInfo) :
	bufferCount_(bufferCount),
	mm_(mm),
	mainWindow_(mainWindow),
	targetFrameInfo_(targetFrameInfo),
	frameCount_(targetFrameInfo[0]),
	volumeCount_(targetFrameInfo[2]),
	currentFrame_(targetFrameInfo[1])
{
	width = mm->GetCameraIntParameter(STREAM_MODULE, "Width");
	height = mm->GetCameraIntParameter(STREAM_MODULE, "Height");

	if (mm->GetCameraStringParameter(REMOTE_MODULE, "AcquisitionMode") == "SingleFrame")
	{
		framerate = frameCount_ * volumeCount_;
	}
	else
	{
		framerate = mm->GetCameraFloatParameter(REMOTE_MODULE, "AcquisitionFrameRate");
	}

	framesToDrop_ = ceilf(framerate / (frameCount_ * MAX_FRAMES_PER_SECOND)) - 1;

	buf_ = new unsigned char[width * height];

	pix_ = new PixmapReadyObject();

	frameThd_ = std::thread(&DisplayThread::CheckFrameInfo, this);
	disThd_ = std::thread(&DisplayThread::Display, this);
}

DisplayThread::~DisplayThread()
{
	delete[] buf_;
	delete pix_;
}

void DisplayThread::CheckFrameInfo()   //This is super questionable, redesign at some point?
{
	while (active)
	{
		if (frameCount_ != targetFrameInfo_[0])
		{
			frameCount_ = targetFrameInfo_[0];
			if (mm_->GetCameraStringParameter(REMOTE_MODULE, "AcquisitionMode") == "SingleFrame")
			{
				framerate = frameCount_ * volumeCount_;
			}
			framesToDrop_ = ceilf(framerate / (frameCount_ * MAX_FRAMES_PER_SECOND)) - 1;
		}

		if (currentFrame_ != targetFrameInfo_[1])
		{
			currentFrame_ = targetFrameInfo_[1];
		}

		if (volumeCount_ != targetFrameInfo_[2])
		{
			volumeCount_ = targetFrameInfo_[2];
			if (mm_->GetCameraStringParameter(REMOTE_MODULE, "AcquisitionMode") == "SingleFrame")
			{
				framerate = frameCount_ * volumeCount_;
			}
		}
	}
}

void DisplayThread::Display()
{
	QObject::connect(pix_, SIGNAL(pixmapReady(const QPixmap&, bool)), mainWindow_, SLOT(updateDisplayFrame(const QPixmap&, bool)));

	unsigned long long bufferSize = std::min(mm_->GetImageBufferSize(), width * height);
	int framesDropped = 0;

	while (active)
	{
		mm_->GetImage();
		//mm_->ApplyCameraMask();

		if (bufferCount_ % frameCount_ == currentFrame_)
		{
			if (framesDropped >= framesToDrop_)
			{
				framesDropped = 0;
				try
				{
					std::memcpy(buf_, mm_->GetImageBuffer(), bufferSize);
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
			else
			{
				++framesDropped;
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
	frameThd_.join();
	disThd_.join();
}
