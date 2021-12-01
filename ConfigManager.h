#pragma once

#include "MicroscopeManager.h"
#include "ui_MicroscopeManagerGUI.h"
#include "ui_ConfigDialog.h"

#define SLICES_PER_VOLUME '1'
#define VOLUMES_PER_SECOND '2'
#define VOLUME_SCALE_MIN '3'
#define VOLUME_SCALE_MAX '4'
#define ODORANT_ORDER '5'
#define STATE_ORDER '6'
#define LASER_MODE '7'
#define SCANNER_AMPLITUDE '8'

class ConfigManager : public QObject
{
	Q_OBJECT

public:
	ConfigManager();
	void WriteConfigFile(std::string filename, Ui::ConfigDialog* cfg);
	void ReadConfigFile(std::string filename, MicroscopeManager* mm, Ui::MicroscopeManagerGUIClass* ui, QObject* mainWindow);
	void GetExperimentSettings(double* vsMin, double* vsMax, int* fpv, int* vps, int *lm, double* sa, std::string* exp);

signals:
	void serialDeviceReady(std::string, std::string, int, std::vector<std::string>, std::vector<std::string> startCommands);

private:
	double volumeScaleMin;
	double volumeScaleMax;
	int framesPerVolume;
	int volumesPerSecond;
	int laserMode;
	double scannerAmplitude;
	std::string experimentDevice;
};

