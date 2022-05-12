#include "ProducerDisplayThread.h"

#define MAX_FRAMES_PER_SECOND 30

ProducerDisplayThread::ProducerDisplayThread(int bufsize, MicroscopeManager* mm, QObject* mainWindow, int* targetFrameInfo):
	ProducerThread(bufsize, mm),
	mainWindow_(mainWindow),
	targetFrameInfo_(targetFrameInfo),
	frameCount_(targetFrameInfo[0]),
	currentFrame_(targetFrameInfo[1]),
	bufferCount_(0),
	endDisplay_(false)
{
	width = mm->GetCameraIntParameter(STREAM_MODULE, "Width");
	height = mm->GetCameraIntParameter(STREAM_MODULE, "Height");
	framerate = mm->GetCameraFloatParameter(REMOTE_MODULE, "AcquisitionFrameRate");

	framesToDrop_ = ceilf(framerate / (frameCount_ * MAX_FRAMES_PER_SECOND)) - 1;

	disBuf_ = new unsigned char[width * height];
	pix_ = new PixmapReadyObject();
}

ProducerDisplayThread::~ProducerDisplayThread()
{}

void ProducerDisplayThread::StartThreads()
{
	StartThread();
	frameThd_ = std::thread(&ProducerDisplayThread::CheckFrameInfo, this);
	disThd_ = std::thread(&ProducerDisplayThread::Display, this);
}

void ProducerDisplayThread::WaitForThread()
{
	{
		std::unique_lock<std::mutex> ul(pMutex);
		endDisplay_ = true;
	}
	pCV.notify_all();

	frameThd_.join();
	disThd_.join();
	ProducerThread::WaitForThread();
}

void ProducerDisplayThread::CheckFrameInfo()
{
	while (active)
	{
		if (frameCount_ != targetFrameInfo_[0])
		{
			frameCount_ = targetFrameInfo_[0];
			framesToDrop_ = ceilf(framerate / (frameCount_ * MAX_FRAMES_PER_SECOND)) - 1;
		}

		if (currentFrame_ != targetFrameInfo_[1])
		{
			currentFrame_ = targetFrameInfo_[1];
		}
	}
}

void ProducerDisplayThread::Display()
{
	QObject::connect(pix_, SIGNAL(pixmapReady(const QPixmap&, bool)), mainWindow_, SLOT(updateDisplayFrame(const QPixmap&, bool)));

	int targetFrame;
	int framesDropped = 0;

	while (active)
	{
		if (currentFrame_ < bufReadys.size())
		{
			targetFrame = currentFrame_;
		}
		else
		{
			targetFrame = 0;
		}
		if (framesDropped >= framesToDrop_)
		{
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
		else
		{
			++framesDropped;
		}
	}
}

void ProducerDisplayThread::processPixmap()
{
	pixmapProcessed = true;
}