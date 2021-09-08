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

class ConfigManager
{
public:
	ConfigManager();
	void WriteConfigFile(std::string filename, Ui::ConfigDialog* cfg);
	void ReadConfigFile(std::string filename, MicroscopeManager* mm, Ui::MicroscopeManagerGUIClass* ui);
	void GetExperimentSettings(double* vsMin, double* vsMax, int* fpv, int* vps, std::string* exp);

private:
	double volumeScaleMin;
	double volumeScaleMax;
	int framesPerVolume;
	int volumesPerSecond;
	std::string experimentDevice;
};

