#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_MicroscopeManagerGUI.h"
#include "MicroscopeManager.h"
#include "AcquisitionDisplayThread.h"
#include <RangeSlider.h>
#include "SerialQueueObject.h"

class MicroscopeManagerGUI : public QMainWindow
{
    Q_OBJECT

public:
    MicroscopeManagerGUI(QWidget *parent = Q_NULLPTR);
    ~MicroscopeManagerGUI();

public slots:
    void updateDisplayFrame(const QPixmap & pixmap, bool acq);
    void connectSerialDevice(std::string deviceName, std::string port, int baudrate, std::vector<std::string> exitCommands);
    void connectSerialDevice(std::string deviceName, std::string port, int baudrate, std::vector<std::string> exitCommands, std::vector<std::string> startCommands);
    void readFromSerialDevice(std::string message);

private slots:
    void writeConfig();
    void readConfig();

    void snapImage();
    void acquireHelper();
    void acquireStart();
    void acquireStop();
    void liveHelper();
    void liveStart();
    void liveStop();
    void experimentSetup();
    void startExperiment();
    void stopExperiment();
    void setTargetFrame();

    void setFilename();
    void openConnectSerialDialog();
    void disconnectSerialDevice();
    void writeToSerialDevice();
    void moveScrollBarToBottom(int min, int max);

    void setVolumeScaleSliderMin();
    void setVolumeScaleSliderMax();
    void setVolumeScaleMin();
    void setVolumeScaleMax();
    void setFramesPerVolume();
    void setVolumesPerSecond();
    void setLaserMode();
    void setScannerAmplitude();

    void addOdorant();
    void shuffleOdorants();
    void addState();

private:
    void startupMenu();
    void defaultStartup();
    void writeExperimentParameter(char parameter, std::string data);

    Ui::MicroscopeManagerGUIClass ui;
    MicroscopeManager* mm;
    std::string configFile;
    unsigned char * buf;
    QImage img;
    unsigned long long width;
    unsigned long long height;
    MMThread* cameraThd;
    bool acquiring;
    bool experimentActive;
    bool fileOpened;
    std::string filepath;
    int imageCount;
    std::map<std::string, MMThread*> serialThds;
    SerialQueueObject* serialQueue;

    RangeSlider* volumeScale;
    double volumeScaleMin;
    double volumeScaleMax;
    int framesPerVolume;
    int volumesPerSecond;
    int laserMode;
    float laserPower;
    double scannerAmplitude;
    int* targetFrameInfo;
    std::string experimentSettingsDevice;
    std::string experimentDescription;
    std::vector<std::pair<char, int>> stateAndDuration;
    std::vector<char> odorants;
};
