#pragma once

#include "MMThread.h"
#include "MicroscopeManager.h"
#include "qwidget.h"
#include "PixmapReadyObject.h"

class AcquisitionDisplayThread :
	public MMThread
{
public:
	AcquisitionDisplayThread(unsigned long long bufferCount, MicroscopeManager* mm, QObject* mainWindow, int* targetFrameInfo);
	AcquisitionDisplayThread(unsigned long long bufferCount, MicroscopeManager* mm, QObject* mainWindow, int* targetFrameInfo, std::vector<std::pair<char, int>> sd, std::vector<char> o, bool expActive, int fpv, int vps, float vsmin, float vsmax, int lm, float lp,  std::string ed);
	~AcquisitionDisplayThread();
	void WaitForThread();
	void processPixmap();

private:
	void Acquire();
	void Display();

	unsigned long long bufferCount_;
	std::thread acqThd_;
	std::thread disThd_;
	MicroscopeManager* mm_;
	unsigned char* buf_;
	QObject* mainWindow_;
	int* targetFrameInfo_;
	unsigned long long width;
	unsigned long long height;
	std::atomic_bool pixmapProcessed;
	PixmapReadyObject* pix_;
	std::atomic_bool masked;

	//Metadata info
	bool experimentActive;
	int laserMode;
	int framesPerVolume;
	int volumesPerSecond;
	std::string experimentDescription;
	float volumeScaleMin;
	float volumeScaleMax;
	float laserPower;
	int cameraMode;
	std::vector<std::pair<char, int>> stateAndDuration;
	std::vector<char> odorants;

	int stateIndex;
	int odorantIndex;
	char currentOdor;
	char currentLaser;
	int remainingLaserImages;
	int remainingStateImages;
};

