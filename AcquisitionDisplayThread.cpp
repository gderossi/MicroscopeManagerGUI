#include "AcquisitionDisplayThread.h"
#include "MicroscopeManagerGUI.h"
#include <chrono>

AcquisitionDisplayThread::AcquisitionDisplayThread(unsigned long long bufferCount, MicroscopeManager* mm, QObject* mainWindow, int* targetFrameInfo) :
	bufferCount_(bufferCount),
	mm_(mm),
	mainWindow_(mainWindow),
	targetFrameInfo_(targetFrameInfo),
	experimentActive(false),
	framesPerVolume(0),
	volumesPerSecond(0),
	volumeScaleMin(0),
	volumeScaleMax(0),
	laserMode(0),
	laserPower(0),
	cameraMode(1)
{
	width = mm->GetCameraIntParameter(STREAM_MODULE, "Width");
	height = mm->GetCameraIntParameter(STREAM_MODULE, "Height");
	buf_ = new unsigned char[width * height];

	pix_ = new PixmapReadyObject();

	acqThd_ = std::thread(&AcquisitionDisplayThread::Acquire, this);
	disThd_ = std::thread(&AcquisitionDisplayThread::Display, this);
}

AcquisitionDisplayThread::AcquisitionDisplayThread(unsigned long long bufferCount, MicroscopeManager* mm, QObject* mainWindow, int* targetFrameInfo, std::vector<std::pair<char, int>> sd, std::vector<char> o, bool expActive=false, int fpv=0, int vps=0, float vsmin=0, float vsmax=0, int lm=0, float lp=0, std::string ed="") :
	bufferCount_(bufferCount),
	mm_(mm),
	mainWindow_(mainWindow),
	targetFrameInfo_(targetFrameInfo),
	experimentActive(expActive),
	framesPerVolume(fpv),
	volumesPerSecond(vps),
	volumeScaleMin(vsmin),
	volumeScaleMax(vsmax),
	laserMode(lm),
	laserPower(lp),
	stateAndDuration(sd),
	odorants(o),
	experimentDescription(ed),
	cameraMode(1)
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
	MICROSCOPE_METADATA* metadata = (MICROSCOPE_METADATA*)VirtualAlloc(NULL, 512, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (metadata) {
		metadata->planeCount = framesPerVolume;
		metadata->volumeRange = volumeScaleMax - volumeScaleMin;
		metadata->laserPower = laserPower;
		metadata->cameraMode = cameraMode;
		strcpy(metadata->sampleInfo, experimentDescription.c_str());

		if (experimentActive)
		{
			
			if (stateAndDuration.size() > 0)
			{
				stateIndex = 0;
				remainingStateImages = stateAndDuration[stateIndex].second * framesPerVolume * volumesPerSecond;
			}
			if (odorants.size() > 0)
			{
				odorantIndex = 0;
				currentOdor = (odorants[odorantIndex] << 4) + stateAndDuration[stateIndex].first;
				metadata->currentOdor = currentOdor;
			}
			if (laserMode == 0)
			{
				currentLaser = 1;
				metadata->currentLaser = currentLaser;
				remainingLaserImages = framesPerVolume;
			}
			else if (laserMode == 1)
			{
				metadata->currentLaser = 1;
			}
			else if (laserMode == 2)
			{
				metadata->currentLaser = 2;
			}
		}
		else
		{
			metadata->currentLaser = 0;
			metadata->currentOdor = 0;
		}
	}
	//unsigned char* metadataBuffer = (unsigned char*)VirtualAlloc(NULL, 512, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	else
	{
		active = false;
	}
	while (active)
	{
		try
		{
			masked = false;
			mm_->GetImage();
			mm_->ApplyCameraMask();
			masked = true;

			const auto p1 = std::chrono::system_clock::now();
			metadata->timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(p1.time_since_epoch()).count();
			
			if (experimentActive)
			{

				if (laserMode == 0)
				{
					remainingLaserImages--;
					if (remainingLaserImages == 0)
					{
						remainingLaserImages = framesPerVolume;

						if (currentLaser == 1)
						{
							currentLaser = 2;
						}
						else
						{
							currentLaser = 1;
						}

						metadata->currentLaser = currentLaser;
					}
				}
			}
			//memcpy(metadataBuffer, metadata, 512);

			mm_->WriteFile((unsigned char*)metadata, 512);
			mm_->WriteFile(mm_->GetImageBuffer(), mm_->GetImageBufferSize());

			remainingStateImages--;
			if (remainingStateImages == 0)
			{
				stateIndex++;
				if (stateIndex == stateAndDuration.size())
				{
					stateIndex = 0;
					odorantIndex++;
				}
				if (odorantIndex == odorants.size())
				{
					active = false;
				}
				else
				{
					remainingStateImages = stateAndDuration[stateIndex].second * framesPerVolume * volumesPerSecond;
					currentOdor = (odorants[odorantIndex] << 4) + stateAndDuration[stateIndex].first;
					metadata->currentOdor = currentOdor;
				}
			}
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
				while (!masked) {}
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
