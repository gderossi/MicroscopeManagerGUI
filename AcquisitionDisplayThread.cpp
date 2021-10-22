#include "AcquisitionDisplayThread.h"

AcquisitionDisplayThread::AcquisitionDisplayThread(unsigned long long bufferCount, MicroscopeManager* mm, QLabel* displayFrame, int* targetFrameInfo) :
	bufferCount_(bufferCount),
	mm_(mm),
	displayFrame_(displayFrame),
	targetFrameInfo_(targetFrameInfo)
{
	width = mm->GetCameraIntParameter(STREAM_MODULE, "Width");
	height = mm->GetCameraIntParameter(STREAM_MODULE, "Height");
	buf_ = new unsigned char[width * height];

	acqThd_ = std::thread(&AcquisitionDisplayThread::Acquire, this);
	disThd_ = std::thread(&AcquisitionDisplayThread::Display, this);
}

AcquisitionDisplayThread::~AcquisitionDisplayThread()
{
	delete buf_;
}

void AcquisitionDisplayThread::Acquire()
{
	while (active)
	{
		mm_->GetImage();
		mm_->WriteFile(mm_->GetImageBuffer(), mm_->GetImageBufferSize());

		if (--bufferCount_ == 0)
		{
			active = false;
		}
	}
}

void AcquisitionDisplayThread::Display()
{
	while (active)
	{
		if (bufferCount_ % targetFrameInfo_[0] == targetFrameInfo_[1])
		{
			std::memcpy(buf_, mm_->GetImageBuffer(), mm_->GetImageBufferSize());
			img_ = QImage(buf_, width, height, width, QImage::Format_Grayscale8, NULL, NULL);
			displayFrame_->setPixmap(QPixmap::fromImage(img_));
		}
	}
}

void AcquisitionDisplayThread::WaitForThread()
{
	acqThd_.join();
}
