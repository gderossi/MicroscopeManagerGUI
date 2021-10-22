#include "MicroscopeManagerGUI.h"
#include "AcquisitionDisplayThread.h"
#include "DisplayThread.h"
#include "SerialConsoleThread.h"
#include "ConfigManager.h"
#include "ConfigDialog.h"
#include "ConnectSerialDeviceDialog.h"
#include "OdorantConfigBox.h"
#include "StateConfigBox.h"
#include <QtWidgets>
#include <algorithm>
#include <random>

#define DEFAULT_FRAMES_PER_VOLUME 20
#define DEFAULT_VOLUMES_PER_SECOND 20
#define VOLUME_SCALE_SLIDER_MULTIPLIER 1000
#define START_EXPERIMENT_DEVICE '0'

MicroscopeManagerGUI::MicroscopeManagerGUI(QWidget* parent) :
    QMainWindow(parent),
    mm(new MicroscopeManager("D:/test")),
    buf(NULL),
    cameraThd(NULL),
    acquiring(false),
    experimentActive(false),
    fileOpened(false),
    filepath(""),
    imageCount(0),
    volumeScaleMin(0),
    volumeScaleMax(1),
    framesPerVolume(DEFAULT_FRAMES_PER_VOLUME),
    volumesPerSecond(DEFAULT_VOLUMES_PER_SECOND)
{
    ui.setupUi(this);

    ui.odorantOrderScrollAreaContents->setLayout(new QVBoxLayout());
    ui.stateOrderScrollAreaContents->setLayout(new QVBoxLayout());

    startupMenu();

    width = mm->GetCameraIntParameter(STREAM_MODULE, "Width");
    height = mm->GetCameraIntParameter(STREAM_MODULE, "Height");

    buf = new unsigned char[width * height];

    ui.experimentButtonsFrame->setVisible(false);

    volumeScale = new RangeSlider(Qt::Horizontal, RangeSlider::Option::DoubleHandles, nullptr);
    ui.experimentSettingsInnerLayout->insertWidget(1, volumeScale);
    ui.experimentSettingsFrame->setLayout(ui.experimentSettingsInnerLayout);

    volumeScale->SetMinimum(0);
    volumeScale->SetMaximum(VOLUME_SCALE_SLIDER_MULTIPLIER);
    volumeScale->SetLowerValue(volumeScaleMin* VOLUME_SCALE_SLIDER_MULTIPLIER);
    volumeScale->SetUpperValue(volumeScaleMax* VOLUME_SCALE_SLIDER_MULTIPLIER);
    ui.volumeMin->setValue(volumeScaleMin);
    ui.volumeMax->setValue(volumeScaleMax);
    ui.volumeMin->setMaximum(volumeScaleMax);
    ui.volumeMax->setMinimum(volumeScaleMin);
    ui.framesPerVolumeSpinBox->setValue(framesPerVolume);
    ui.volumesPerSecondSpinBox->setValue(volumesPerSecond);
    ui.frameSelectSlider->setMaximum(framesPerVolume-1);

    targetFrameInfo = new int[2];
    targetFrameInfo[0] = framesPerVolume;
    targetFrameInfo[1] = 0;

    //File menu
    connect(ui.action_Filename, &QAction::triggered, this, &MicroscopeManagerGUI::setFilename);
    connect(ui.actionNew_Config, &QAction::triggered, this, &MicroscopeManagerGUI::writeConfig);

    //Serial menu
    connect(ui.actionConnect, &QAction::triggered, this, &MicroscopeManagerGUI::connectSerialDevice);
    connect(ui.actionDisconnect, &QAction::triggered, this, &MicroscopeManagerGUI::disconnectSerialDevice);

    //Serial console
    connect(ui.consoleEnter, &QPushButton::clicked, this, &MicroscopeManagerGUI::writeToSerialDevice);

    //Camera control
    connect(ui.snapImageButton, &QPushButton::clicked, this, &MicroscopeManagerGUI::snapImage);
    connect(ui.acquireButton, &QPushButton::clicked, this, &MicroscopeManagerGUI::acquireHelper);
    connect(ui.liveViewButton, &QPushButton::clicked, this, &MicroscopeManagerGUI::liveHelper);
    connect(ui.setupExperimentButton, &QPushButton::clicked, this, &MicroscopeManagerGUI::experimentSetup);
    connect(ui.startExperimentButton, &QPushButton::clicked, this, &MicroscopeManagerGUI::startExperiment);
    connect(ui.stopExperimentButton, &QPushButton::clicked, this, &MicroscopeManagerGUI::stopExperiment);
    connect(ui.frameSelectSlider, &QSlider::valueChanged, this, &MicroscopeManagerGUI::setTargetFrame);

    //Experiment settings
    connect(volumeScale, &RangeSlider::lowerValueChanged, this, &MicroscopeManagerGUI::setVolumeScaleMin);
    connect(volumeScale, &RangeSlider::upperValueChanged, this, &MicroscopeManagerGUI::setVolumeScaleMax);
    connect(ui.volumeMin, &QDoubleSpinBox::valueChanged, this, &MicroscopeManagerGUI::setVolumeScaleSliderMin);
    connect(ui.volumeMax, &QDoubleSpinBox::valueChanged, this, &MicroscopeManagerGUI::setVolumeScaleSliderMax);
    connect(ui.framesPerVolumeSpinBox, &QSpinBox::valueChanged, this, &MicroscopeManagerGUI::setFramesPerVolume);
    connect(ui.volumesPerSecondSpinBox, &QSpinBox::valueChanged, this, &MicroscopeManagerGUI::setVolumesPerSecond);

    connect(ui.addOdorantButton, &QToolButton::clicked, this, &MicroscopeManagerGUI::addOdorant);
    connect(ui.shuffleOdorantsButton, &QToolButton::clicked, this, &MicroscopeManagerGUI::shuffleOdorants);
    connect(ui.addStateButton, &QToolButton::clicked, this, &MicroscopeManagerGUI::addState);
}

MicroscopeManagerGUI::~MicroscopeManagerGUI()
{
    if (cameraThd)
    {
        cameraThd->StopThread();
        cameraThd->WaitForThread();
        delete cameraThd;
    }

    delete buf;
    delete mm;
    delete volumeScale;
    delete targetFrameInfo;
}

void MicroscopeManagerGUI::writeConfig()
{
    ConfigDialog cfgDialog(mm->imageManagerTypes, mm->cameraManagerTypes, mm->serialManagerTypes, mm->GetSerialPorts());
    cfgDialog.exec();
}

void MicroscopeManagerGUI::readConfig()
{
    ConfigManager cfg;
    configFile = QFileDialog::getOpenFileName().toUtf8().constData();
    if (configFile == "")
    {
        defaultStartup();
    }
    else
    {
        cfg.ReadConfigFile(configFile, mm, &ui);
        cfg.GetExperimentSettings(&volumeScaleMin, &volumeScaleMax, &framesPerVolume, &volumesPerSecond, &experimentSettingsDevice);
    }
}

void MicroscopeManagerGUI::snapImage()
{
    if (filepath == "")
    {
        setFilename();
    }

    std::string imageCountFilepath = filepath + "_" + std::to_string(imageCount);
    imageCount++;

    mm->SetFilename(imageCountFilepath);
    mm->CreateFile();

    mm->SnapImage();
    std::memcpy(buf, mm->GetImageBuffer(), mm->GetImageBufferSize());
    img = QImage(buf, width, height, width, QImage::Format_Grayscale8, NULL, NULL);
    ui.imageDisplay->setPixmap(QPixmap::fromImage(img));

    mm->WriteFile(buf, width * height);
    mm->CloseFile();
}

void MicroscopeManagerGUI::acquireHelper()
{
    if (!acquiring)
    {
        acquireStart();
    }
    else
    {
        acquireStop();
    }
}

void MicroscopeManagerGUI::acquireStart()
{
    if (filepath == "")
    {
        setFilename();
    }

    ui.acquireButton->setText("Stop Acquisition");
    ui.liveViewButton->setEnabled(false);
    ui.snapImageButton->setEnabled(false);
    ui.setupExperimentButton->setEnabled(false);

    mm->SetFilename(filepath);
    mm->CreateFile();

    mm->StartAcquisition(GENTL_INFINITE);
    cameraThd = new AcquisitionDisplayThread(GENTL_INFINITE, mm, ui.imageDisplay, targetFrameInfo);
    acquiring = true;
}

void MicroscopeManagerGUI::acquireStop()
{
    if (cameraThd)
    {
        cameraThd->StopThread();
        cameraThd->WaitForThread();
        delete cameraThd;
        cameraThd = NULL;
    }

    mm->StopAcquisition();
    mm->CloseFile();

    ui.acquireButton->setText("Start Acquisition");
    ui.liveViewButton->setEnabled(true);
    ui.snapImageButton->setEnabled(true);
    ui.setupExperimentButton->setEnabled(true);
    acquiring = false;
}

void MicroscopeManagerGUI::liveHelper()
{
    if (!acquiring)
    {
        liveStart();
    }
    else
    {
        liveStop();
    }
}

void MicroscopeManagerGUI::liveStart()
{
    ui.liveViewButton->setText("Stop Live View");
    ui.snapImageButton->setEnabled(false);
    ui.acquireButton->setEnabled(false);
    ui.setupExperimentButton->setEnabled(false);

    mm->StartAcquisition(GENTL_INFINITE);
    cameraThd = new DisplayThread(GENTL_INFINITE, mm, ui.imageDisplay, targetFrameInfo);
    acquiring = true;
}

void MicroscopeManagerGUI::liveStop()
{
    if (cameraThd)
    {
        cameraThd->StopThread();
        cameraThd->WaitForThread();
        delete cameraThd;
        cameraThd = NULL;
    }

    mm->StopAcquisition();

    ui.liveViewButton->setText("Live View");
    ui.snapImageButton->setEnabled(true);
    ui.acquireButton->setEnabled(true);
    ui.setupExperimentButton->setEnabled(true);
    acquiring = false;
}

void MicroscopeManagerGUI::setFilename()
{
    QString qDir = QFileDialog::getSaveFileName();
    filepath = qDir.toUtf8().constData();

    std::string filepathText = "Filepath: " + filepath;
    ui.filepath->setText(QString(filepathText.c_str()));

    imageCount = 0;
}

void MicroscopeManagerGUI::connectSerialDevice()
{
    ConnectSerialDeviceDialog serialDialog(mm->GetSerialPorts(), mm, ui.consoleDeviceList);
    serialDialog.exec();
}

void MicroscopeManagerGUI::disconnectSerialDevice()
{
    std::string deviceName;
    int i;

    deviceName = ui.consoleDeviceList->currentText().toUtf8().constData();
    i = ui.consoleDeviceList->currentIndex();

    mm->DisconnectSerialDevice(deviceName);

    ui.consoleDeviceList->removeItem(i);
}

void MicroscopeManagerGUI::writeToSerialDevice()
{
    if (ui.consoleInput->text() != "" && ui.consoleDeviceList->currentText() != "")
    {
        std::string deviceName;
        std::string data;
        std::string output;
        unsigned long long writeSize;

        deviceName = ui.consoleDeviceList->currentText().toUtf8().constData();
        data = ui.consoleInput->text().toUtf8().constData();

        if (data == "EXPERIMENT_SETTINGS")
        {
            experimentSettingsDevice = deviceName;
            output = ">> Set " + deviceName + " to experiment device\n";
            output = ui.consoleOutput->text().toUtf8().constData() + output;
            ui.consoleOutput->setText(output.c_str());
        }
        else
        {
            output = data;
            data += '\r';
            writeSize = data.size();

            mm->SerialWrite(deviceName, data.c_str(), writeSize);

            ui.consoleInput->setText("");

            output = ">> " + output + '\n';
            output = ui.consoleOutput->text().toUtf8().constData() + output;
            ui.consoleOutput->setText(output.c_str());

            readFromSerialDevice();
        }
    }
}

void MicroscopeManagerGUI::readFromSerialDevice()
{
    std::string deviceName;
    std::string output;
    unsigned long long readSize = 128;

    deviceName = ui.consoleDeviceList->currentText().toUtf8().constData();

    output = mm->SerialRead(deviceName, readSize);
    output += '\n';
    output = ui.consoleOutput->text().toUtf8().constData() + output;
    ui.consoleOutput->setText(output.c_str());
}

void MicroscopeManagerGUI::setVolumeScaleSliderMin()
{
    volumeScaleMin = ui.volumeMin->value();
    volumeScale->setLowerValue(volumeScaleMin*VOLUME_SCALE_SLIDER_MULTIPLIER);
    ui.volumeMax->setMinimum(volumeScaleMin);
}

void MicroscopeManagerGUI::setVolumeScaleSliderMax()
{
    volumeScaleMax = ui.volumeMax->value();
    volumeScale->setUpperValue(volumeScaleMax * VOLUME_SCALE_SLIDER_MULTIPLIER);
    ui.volumeMin->setMaximum(volumeScaleMax);
}

void MicroscopeManagerGUI::setVolumeScaleMin()
{
    volumeScaleMin = ((double)volumeScale->GetLowerValue()) / VOLUME_SCALE_SLIDER_MULTIPLIER;
    ui.volumeMin->setValue(volumeScaleMin);

    std::string data = std::to_string(volumeScaleMin);
    writeExperimentParameter(VOLUME_SCALE_MIN, data);
}

void MicroscopeManagerGUI::setVolumeScaleMax()
{
    volumeScaleMax = ((double)volumeScale->GetUpperValue()) / VOLUME_SCALE_SLIDER_MULTIPLIER;
    ui.volumeMax->setValue(volumeScaleMax);

    std::string data = std::to_string(volumeScaleMax);
    writeExperimentParameter(VOLUME_SCALE_MAX, data);
}

void MicroscopeManagerGUI::setFramesPerVolume()
{
    framesPerVolume = ui.framesPerVolumeSpinBox->value();
    ui.frameSelectSlider->setMaximum(framesPerVolume-1);
    targetFrameInfo[0] = framesPerVolume;

    std::string data = std::to_string(framesPerVolume);
    writeExperimentParameter(SLICES_PER_VOLUME, data);
}

void MicroscopeManagerGUI::setVolumesPerSecond()
{
    volumesPerSecond = ui.volumesPerSecondSpinBox->value();

    std::string data = std::to_string(volumesPerSecond);
    writeExperimentParameter(VOLUMES_PER_SECOND, data);
}

void MicroscopeManagerGUI::setTargetFrame()
{
    targetFrameInfo[1] = ui.frameSelectSlider->value();
}

void MicroscopeManagerGUI::experimentSetup()
{
    if (!acquiring)
    {
        if (experimentSettingsDevice != "")
        {
            std::string startCommand = "" + START_EXPERIMENT_DEVICE + '\r';
            mm->SerialWrite(experimentSettingsDevice, startCommand.c_str(), startCommand.size());
        }

        if (filepath == "")
        {
            setFilename();
        }

        mm->SetFilename(filepath);
        mm->CreateFile();
        mm->StartAcquisition(GENTL_INFINITE);
        cameraThd = new DisplayThread(GENTL_INFINITE, mm, ui.imageDisplay, targetFrameInfo);

        ui.cameraButtonsFrame->setVisible(false);
        ui.experimentButtonsFrame->setVisible(true);
        acquiring = true;
    }
}

void MicroscopeManagerGUI::startExperiment()
{
    if (!experimentActive)
    {
        if (cameraThd)
        {
            cameraThd->StopThread();
            cameraThd->WaitForThread();
            delete cameraThd;
        }

        cameraThd = new AcquisitionDisplayThread(GENTL_INFINITE, mm, ui.imageDisplay, targetFrameInfo);

        if (experimentSettingsDevice != "")
        {
            std::string startCommand = "" + START_EXPERIMENT_DEVICE + '\r';
            mm->SerialWrite(experimentSettingsDevice, startCommand.c_str(), startCommand.size());
        }
    }
}

void MicroscopeManagerGUI::stopExperiment()
{
    acquiring = false;
    experimentActive = false;

    if (cameraThd)
    {
        cameraThd->StopThread();
        cameraThd->WaitForThread();
        delete cameraThd;
        cameraThd = NULL;
    }

    mm->StopAcquisition();
    mm->CloseFile();

    ui.experimentButtonsFrame->setVisible(false);
    ui.cameraButtonsFrame->setVisible(true);
}

void MicroscopeManagerGUI::addOdorant()
{
    OdorantConfigBox* o = new OdorantConfigBox(ui.odorantOrderScrollAreaContents);
    ui.odorantOrderScrollAreaContents->layout()->addWidget(o);
}

void MicroscopeManagerGUI::shuffleOdorants()
{
    std::vector<int> odorants;
    QObjectList odorantList = ui.odorantOrderScrollAreaContents->children();

    if (odorantList.size() > 1)
    {
        for (int i = 1; i < odorantList.size(); i++)
        {
            OdorantConfigBox* o = (OdorantConfigBox*)odorantList[i];
            odorants.push_back(o->getUi()->odorantComboBox->currentData().toInt());
        }

        std::shuffle(odorants.begin(), odorants.end(), std::default_random_engine());

        for (int i = 1; i < odorantList.size(); i++)
        {
            OdorantConfigBox* o = (OdorantConfigBox*)odorantList[i];
            o->getUi()->odorantComboBox->setCurrentIndex(odorants[i-1] - 1);
        }
    }
}

void MicroscopeManagerGUI::addState()
{
    StateConfigBox* s = new StateConfigBox(ui.stateOrderScrollAreaContents);
    ui.stateOrderScrollAreaContents->layout()->addWidget(s);
}


void MicroscopeManagerGUI::writeExperimentParameter(char parameter, std::string data)
{
    data = parameter + data + '\r';
    unsigned long long writeSize = data.size();

    mm->SerialWrite(experimentSettingsDevice, data.c_str(), writeSize);
}

void MicroscopeManagerGUI::startupMenu()
{
    QMessageBox::StandardButton reply = QMessageBox::question(this, "Welcome", "Wecome to Microscope Manager. Would you like to load a configuration?", QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes)
    {
        readConfig();
    }
    else
    {
        defaultStartup();
    }
}

void MicroscopeManagerGUI::defaultStartup()
{
    mm->CreateImageManager("Raw", NULL);
    mm->CreateCameraManager("FLIR", NULL);
    mm->CreateSerialManager("Windows", NULL);
}
