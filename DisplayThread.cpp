#include "DisplayThread.h"

DisplayThread::DisplayThread(unsigned long long bufferCount, MicroscopeManager* mm, QLabel* displayFrame, int* targetFrameInfo) :
	bufferCount_(bufferCount),
	mm_(mm),
	displayFrame_(displayFrame),
	targetFrameInfo_(targetFrameInfo)
{
	width = mm->GetCameraIntParameter(STREAM_MODULE, "Width");
	height = mm->GetCameraIntParameter(STREAM_MODULE, "Height");
	buf_ = new unsigned char[width * height];

	disThd_ = std::thread(&DisplayThread::Display, this);
}

DisplayThread::~DisplayThread()
{
	delete buf_;
}

void DisplayThread::Display()
{
	while (active)
	{
		if (bufferCount_ % targetFrameInfo_[0] == targetFrameInfo_[1])
		{
			mm_->GetImage();
			std::memcpy(buf_, mm_->GetImageBuffer(), mm_->GetImageBufferSize());
			img_ = QImage(buf_, width, height, width, QImage::Format_Grayscale8, NULL, NULL);
			displayFrame_->setPixmap(QPixmap::fromImage(img_));
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
