#include "AcquisitionDisplayThread.h"

AcquisitionDisplayThread::AcquisitionDisplayThread(unsigned long long bufferCount, MicroscopeManager* mm, QLabel* displayFrame, int* targetFrameInfo) :
	bufferCount_(bufferCount),
	mm_(mm),
	displayFrame_(displayFrame),
	buf_(new unsigned char[1920*1080]),
	acqThd_(&AcquisitionDisplayThread::Acquire, this),
	disThd_(&AcquisitionDisplayThread::Display, this),
	targetFrameInfo_(targetFrameInfo)
{}

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
			img_ = QImage(buf_, 1920, 1080, 1920, QImage::Format_Grayscale8, NULL, NULL);
			displayFrame_->setPixmap(QPixmap::fromImage(img_));
		}
	}
}

void AcquisitionDisplayThread::WaitForThread()
{
	acqThd_.join();
}
