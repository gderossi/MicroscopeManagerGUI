#include "MicroscopeManagerGUI.h"
#include "AcquisitionDisplayThread.h"
#include "DisplayThread.h"
#include "SerialDeviceThread.h"
#include "ConfigManager.h"
#include "ConfigDialog.h"
#include "ConnectSerialDeviceDialog.h"
#include "OdorantConfigBox.h"
#include "StateConfigBox.h"
#include <algorithm>
#include <random>

#define DEFAULT_FRAMES_PER_VOLUME 20
#define DEFAULT_VOLUMES_PER_SECOND 20
#define VOLUME_SCALE_SLIDER_MULTIPLIER 1000
#define START_EXPERIMENT_DEVICE '0'
#define LOCATE_EXPERIMENT_DEVICE '?'
#define ABORT_EXPERIMENT '!'

MicroscopeManagerGUI::MicroscopeManagerGUI(QWidget* parent) :
    QMainWindow(parent),
    mm(new MicroscopeManager("D:/test")),
    serialQueue(new SerialQueueObject(this)),
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
    volumesPerSecond(DEFAULT_VOLUMES_PER_SECOND),
    scannerAmplitude(1)
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
    ui.laserModeComboBox->setCurrentIndex(laserMode);
    ui.scannerAmplitudeDoubleSpinBox->setValue(scannerAmplitude);
    ui.frameSelectSlider->setMaximum(framesPerVolume-1);

    targetFrameInfo = new int[2];
    targetFrameInfo[0] = framesPerVolume;
    targetFrameInfo[1] = 0;

    //File menu
    connect(ui.action_Filename, &QAction::triggered, this, &MicroscopeManagerGUI::setFilename);
    connect(ui.actionNew_Config, &QAction::triggered, this, &MicroscopeManagerGUI::writeConfig);

    //Serial menu
    connect(ui.actionConnect, &QAction::triggered, this, &MicroscopeManagerGUI::openConnectSerialDialog);
    connect(ui.actionDisconnect, &QAction::triggered, this, &MicroscopeManagerGUI::disconnectSerialDevice);
    connect(ui.serialScrollArea->verticalScrollBar(), SIGNAL(rangeChanged(int, int)), this, SLOT(moveScrollBarToBottom(int, int)));

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
    connect(ui.laserModeComboBox, &QComboBox::currentIndexChanged, this, &MicroscopeManagerGUI::setLaserMode);
    connect(ui.scannerAmplitudeDoubleSpinBox, & QDoubleSpinBox::valueChanged, this, &MicroscopeManagerGUI::setScannerAmplitude);

    connect(ui.addOdorantButton, &QToolButton::clicked, this, &MicroscopeManagerGUI::addOdorant);
    connect(ui.shuffleOdorantsButton, &QToolButton::clicked, this, &MicroscopeManagerGUI::shuffleOdorants);
    connect(ui.addStateButton, &QToolButton::clicked, this, &MicroscopeManagerGUI::addState);

    //connect(ui.imageDisplay, &QLabel::)
}

MicroscopeManagerGUI::~MicroscopeManagerGUI()
{
    if (cameraThd)
    {
        cameraThd->StopThread();
        cameraThd->WaitForThread();
        delete cameraThd;
    }

    //Delete all serial threads
    std::map<std::string, MMThread*>::iterator i;

    for (i = serialThds.begin(); i != serialThds.end(); i++)
    {
        MMThread* thd = i->second;
        thd->StopThread();
        thd->WaitForThread();
        delete thd;
    }

    delete buf;
    delete mm;
    delete serialQueue;
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
        cfg.ReadConfigFile(configFile, mm, &ui, this);
        cfg.GetExperimentSettings(&volumeScaleMin, &volumeScaleMax, &framesPerVolume, &volumesPerSecond, &laserMode, &scannerAmplitude, &experimentSettingsDevice);
    }
}

void MicroscopeManagerGUI::snapImage()
{
    if (filepath == "")
    {
        setFilename();
        if (filepath == "")
        {
            return;
        }
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
        if (filepath == "")
        {
            return;
        }
    }

    ui.acquireButton->setText("Stop Acquisition");
    ui.liveViewButton->setEnabled(false);
    ui.snapImageButton->setEnabled(false);
    ui.setupExperimentButton->setEnabled(false);

    mm->SetFilename(filepath);
    mm->CreateFile();

    mm->StartAcquisition(GENTL_INFINITE);
    cameraThd = new AcquisitionDisplayThread(GENTL_INFINITE, mm, this, targetFrameInfo);
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
    cameraThd = new DisplayThread(GENTL_INFINITE, mm, this, targetFrameInfo);
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

void MicroscopeManagerGUI::openConnectSerialDialog()
{
    ConnectSerialDeviceDialog serialDialog(mm->GetSerialPorts(), this);
    serialDialog.exec();
}

void MicroscopeManagerGUI::connectSerialDevice(std::string deviceName, std::string port, int baudrate, std::vector<std::string> exitCommands)
{
    mm->ConnectSerialDevice(deviceName, port, baudrate, exitCommands);
    ui.consoleDeviceList->addItem(deviceName.c_str());
    serialThds.emplace(deviceName, new SerialDeviceThread(mm->GetSerialDevice(deviceName), serialQueue));
}

void MicroscopeManagerGUI::connectSerialDevice(std::string deviceName, std::string port, int baudrate, std::vector<std::string> exitCommands, std::vector<std::string> startCommands)
{
    mm->ConnectSerialDevice(deviceName, port, baudrate, exitCommands);
    ui.consoleDeviceList->addItem(deviceName.c_str());
    serialThds.emplace(deviceName, new SerialDeviceThread(mm->GetSerialDevice(deviceName), serialQueue));

    for (std::string command : startCommands)
    {
        mm->SerialWrite(deviceName, command.c_str(), command.size());
    }
}

void MicroscopeManagerGUI::disconnectSerialDevice()
{
    std::string deviceName;
    int i;

    deviceName = ui.consoleDeviceList->currentText().toUtf8().constData();
    i = ui.consoleDeviceList->currentIndex();

    MMThread* serialThread = serialThds.find(deviceName)->second;
    serialThds.erase(deviceName);
    serialThread->StopThread();
    serialThread->WaitForThread();
    delete serialThread;

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
            ui.consoleOutput->adjustSize();
            ui.serialScrollAreaContents->setFixedHeight(ui.consoleOutput->height());
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
            ui.consoleOutput->adjustSize();
            ui.serialScrollAreaContents->setFixedHeight(ui.consoleOutput->height());
        }
    }
}

void MicroscopeManagerGUI::readFromSerialDevice(std::string message)
{
    if (message == "")
    {
        return;
    }
    std::string output;
    output = message;
    output += '\n';
    output = ui.consoleOutput->text().toUtf8().constData() + output;
    ui.consoleOutput->setText(output.c_str());
    ui.consoleOutput->adjustSize();
    ui.serialScrollAreaContents->setFixedHeight(ui.consoleOutput->height());
}

void MicroscopeManagerGUI::moveScrollBarToBottom(int min, int max)
{
    Q_UNUSED(min);

    ui.serialScrollArea->verticalScrollBar()->setValue(max);
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

void MicroscopeManagerGUI::setLaserMode()
{
    laserMode = ui.laserModeComboBox->currentIndex();
}

void MicroscopeManagerGUI::setScannerAmplitude()
{
    scannerAmplitude = ui.scannerAmplitudeDoubleSpinBox->value();

    std::string data = std::to_string(scannerAmplitude);
    writeExperimentParameter(SCANNER_AMPLITUDE, data);
}

void MicroscopeManagerGUI::setTargetFrame()
{
    targetFrameInfo[1] = ui.frameSelectSlider->value();
}

void MicroscopeManagerGUI::experimentSetup()
{
    
    if (!acquiring && !experimentActive)
    {
        std::string command;

        if (experimentSettingsDevice == "")
        {
            command = "";
            command += LOCATE_EXPERIMENT_DEVICE;
            command += '\r';
            serialQueue->qMutex.lock();
            for (std::string device : mm->ListConnectedSerialDevices())
            {
                
                mm->SerialWrite(device, command.c_str(), command.size());
                while (!mm->GetSerialDevice(device)->Available()) {} //Add a timeout to this that returns and says "Could not locate" like below
                if (mm->SerialRead(device, 128) == "EXPERIMENT_DEVICE")
                {
                    experimentSettingsDevice = device;
                    break;
                }
            }
            serialQueue->qMutex.unlock();
            if (experimentSettingsDevice == "")
            {
                //No experiment device found, info popup and return
                QMessageBox::information(this, "", "Could not locate control device, experiment setup aborted");
                return;
            }
        }

        if (filepath == "")
        {
            setFilename();
            if (filepath == "")
            {
                //No filename, info popup and return
                QMessageBox::information(this, "", "No output file selected, experiment setup aborted");
                return;
            }
        }

        //Send experiment settings
        command = "";
        command += SLICES_PER_VOLUME + std::to_string(framesPerVolume) + '\r';
        mm->SerialWrite(experimentSettingsDevice, command.c_str(), command.size());

        command = "";
        command += VOLUMES_PER_SECOND + std::to_string(volumesPerSecond) + '\r';
        mm->SerialWrite(experimentSettingsDevice, command.c_str(), command.size());

        command = "";
        command += VOLUME_SCALE_MIN + std::to_string(volumeScaleMin) + '\r';
        mm->SerialWrite(experimentSettingsDevice, command.c_str(), command.size());

        command = "";
        command += VOLUME_SCALE_MAX + std::to_string(volumeScaleMax) + '\r';
        mm->SerialWrite(experimentSettingsDevice, command.c_str(), command.size());

        command = "";
        command += LASER_MODE + std::to_string(laserMode) + '\r';
        mm->SerialWrite(experimentSettingsDevice, command.c_str(), command.size());

        command = "";
        command += SCANNER_AMPLITUDE + std::to_string(scannerAmplitude) + '\r';
        mm->SerialWrite(experimentSettingsDevice, command.c_str(), command.size());

        command = "";
        command += ODORANT_ORDER;
        QObjectList odorantList = ui.odorantOrderScrollAreaContents->children();
        for (int i = 1; i < odorantList.size(); i++)
        {
            OdorantConfigBox* o = (OdorantConfigBox*)odorantList[i];
            command += std::to_string(o->getUi()->odorantComboBox->currentData().toInt());
            o->getUi()->odorantComboBox->setEnabled(false);
            o->getUi()->deleteOdorant->setEnabled(false);
        }
        command += '\r';
        mm->SerialWrite(experimentSettingsDevice, command.c_str(), command.size());

        command = "";
        command += STATE_ORDER;
        QObjectList stateList = ui.stateOrderScrollAreaContents->children();
        command += std::to_string(stateList.size() - 1);
        if (stateList.size() != 1)
        {
            command += " ";
        }

        for (int i = 1; i < stateList.size(); i++)
        {
            StateConfigBox* s = (StateConfigBox*)stateList[i];
            command += std::to_string(s->getUi()->stateComboBox->currentData().toInt() + s->getUi()->durationSpinBox->value());
            if (i != stateList.size() - 1)
            {
                command += " ";
            }
            s->getUi()->stateComboBox->setEnabled(false);
            s->getUi()->durationSpinBox->setEnabled(false);
            s->getUi()->deleteState->setEnabled(false);
        }
        command += '\r';
        mm->SerialWrite(experimentSettingsDevice, command.c_str(), command.size());

        //Start live camera view
        mm->StartAcquisition(GENTL_INFINITE);
        cameraThd = new DisplayThread(GENTL_INFINITE, mm, this, targetFrameInfo);

        //Start microcontroller with given settings
        command = "";
        command += START_EXPERIMENT_DEVICE;
        command += '\r';
        mm->SerialWrite(experimentSettingsDevice, command.c_str(), command.size());

        //Disable changing odorants and states
        ui.shuffleOdorantsButton->setEnabled(false);
        ui.addOdorantButton->setEnabled(false);
        ui.addStateButton->setEnabled(false);
        ui.laserModeComboBox->setEnabled(false);

        ui.cameraButtonsFrame->setVisible(false);
        ui.experimentButtonsFrame->setVisible(true);
        experimentActive = true;
    }
}

void MicroscopeManagerGUI::startExperiment()
{
    if (experimentActive && !acquiring)
    {
        if (cameraThd)
        {
            cameraThd->StopThread();
            cameraThd->WaitForThread();
            delete cameraThd;
        }

        mm->SetFilename(filepath);
        mm->CreateFile();
        cameraThd = new AcquisitionDisplayThread(GENTL_INFINITE, mm, this, targetFrameInfo);

        if (experimentSettingsDevice != "")
        {
            std::string startCommand = "";
            startCommand += START_EXPERIMENT_DEVICE;
            startCommand += '\r';
            mm->SerialWrite(experimentSettingsDevice, startCommand.c_str(), startCommand.size());
        }

        acquiring = true;
        ui.startExperimentButton->setEnabled(false);

        //Disable changing all other experiment settings
        ui.framesPerVolumeSpinBox->setEnabled(false);
        ui.volumesPerSecondSpinBox->setEnabled(false);
        ui.volumeMax->setEnabled(false);
        ui.volumeMin->setEnabled(false);
        volumeScale->setEnabled(false);
        ui.scannerAmplitudeDoubleSpinBox->setEnabled(false);
    }
}

void MicroscopeManagerGUI::stopExperiment()
{
    acquiring = false;
    experimentActive = false;

    if (experimentSettingsDevice != "")
    {
        std::string stopCommand = "";
        stopCommand += ABORT_EXPERIMENT;
        stopCommand += '\r';
        mm->SerialWrite(experimentSettingsDevice, stopCommand.c_str(), stopCommand.size());
    }

    if (cameraThd)
    {
        cameraThd->StopThread();
        cameraThd->WaitForThread();
        delete cameraThd;
        cameraThd = NULL;
    }

    mm->StopAcquisition();
    mm->CloseFile();

    //Enable changing odorants and states
    ui.shuffleOdorantsButton->setEnabled(true);
    ui.addOdorantButton->setEnabled(true);
    ui.addStateButton->setEnabled(true);
    ui.laserModeComboBox->setEnabled(true);

    ui.framesPerVolumeSpinBox->setEnabled(true);
    ui.volumesPerSecondSpinBox->setEnabled(true);
    ui.volumeMax->setEnabled(true);
    ui.volumeMin->setEnabled(true);
    volumeScale->setEnabled(true);
    ui.scannerAmplitudeDoubleSpinBox->setEnabled(true);

    QObjectList odorantList = ui.odorantOrderScrollAreaContents->children();
    for (int i = 1; i < odorantList.size(); i++)
    {
        OdorantConfigBox* o = (OdorantConfigBox*)odorantList[i];
        o->getUi()->odorantComboBox->setEnabled(true);
        o->getUi()->deleteOdorant->setEnabled(true);
    }


    QObjectList stateList = ui.stateOrderScrollAreaContents->children();
    for (int i = 1; i < stateList.size(); i++)
    {
        StateConfigBox* s = (StateConfigBox*)stateList[i];
        s->getUi()->stateComboBox->setEnabled(true);
        s->getUi()->durationSpinBox->setEnabled(true);
        s->getUi()->deleteState->setEnabled(true);
    }

    ui.startExperimentButton->setEnabled(true);
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

void MicroscopeManagerGUI::updateDisplayFrame(const QPixmap & pixmap, bool acq)
{
   
    ui.imageDisplay->setPixmap(pixmap);
    /*if (acq)
    {
        ((AcquisitionDisplayThread*)cameraThd)->processPixmap();
    }
    else
    {
        return;
    }*/
}


void MicroscopeManagerGUI::writeExperimentParameter(char parameter, std::string data)
{
    if (experimentActive)
    {
        data = parameter + data + '\r';
        unsigned long long writeSize = data.size();
        mm->SerialWrite(experimentSettingsDevice, data.c_str(), writeSize);
    }
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
    mm->CreateCameraManager("Test", NULL);
    mm->CreateSerialManager("Windows", NULL);
}
