#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_MicroscopeManagerGUI.h"
#include "MicroscopeManager.h"
#include "AcquisitionDisplayThread.h"
#include <RangeSlider.h>

class MicroscopeManagerGUI : public QMainWindow
{
    Q_OBJECT

public:
    MicroscopeManagerGUI(QWidget *parent = Q_NULLPTR);
    ~MicroscopeManagerGUI();

private slots:
    void writeConfig();
    void readConfig();
    void snapImage();
    void acquireHelper();
    void acquireStart();
    void acquireStop();
    void setFilename();
    void connectSerialDevice();
    void disconnectSerialDevice();
    void writeToSerialDevice();
    void readFromSerialDevice();
    void setVolumeScaleSliderMin();
    void setVolumeScaleSliderMax();
    void setVolumeScaleMin();
    void setVolumeScaleMax();
    void setFramesPerVolume();
    void setVolumesPerSecond();
    void setTargetFrame();

private:
    void writeExperimentParameter(char parameter, std::string data);


    Ui::MicroscopeManagerGUIClass ui;
    MicroscopeManager* mm;
    std::string configFile;
    unsigned char * buf;
    QImage img;
    MMThread* cameraThd;
    bool acquiring;
    bool fileOpened;
    std::string filepath;
    int imageCount;
    std::map<std::string, MMThread*> serialThds;
    RangeSlider* volumeScale;
    double volumeScaleMin;
    double volumeScaleMax;
    int framesPerVolume;
    int volumesPerSecond;
    int* targetFrameInfo;
    std::string experimentSettingsDevice;
};
