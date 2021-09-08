#include "MicroscopeManagerGUI.h"
#include "AcquisitionDisplayThread.h"
#include "SerialConsoleThread.h"
#include "ConfigManager.h"
#include "ConfigDialog.h"
#include "ConnectSerialDeviceDialog.h"
#include <QtWidgets>

#define DEFAULT_FRAMES_PER_VOLUME 20
#define DEFAULT_VOLUMES_PER_SECOND 20
#define VOLUME_SCALE_SLIDER_MULTIPLIER 1000

MicroscopeManagerGUI::MicroscopeManagerGUI(QWidget* parent) :
    QMainWindow(parent),
    mm(new MicroscopeManager("D:/test")),
    buf(new unsigned char[1920*1080]),
    cameraThd(NULL),
    acquiring(false),
    fileOpened(false),
    filepath(""),
    imageCount(0),
    volumeScaleMin(0),
    volumeScaleMax(1),
    framesPerVolume(DEFAULT_FRAMES_PER_VOLUME),
    volumesPerSecond(DEFAULT_VOLUMES_PER_SECOND)
{
    ui.setupUi(this);
    readConfig();

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
    connect(ui.acquireButton, &QPushButton::toggled, this, &MicroscopeManagerGUI::acquireHelper);
    connect(ui.frameSelectSlider, &QSlider::valueChanged, this, &MicroscopeManagerGUI::setTargetFrame);

    //Experiment settings
    connect(volumeScale, &RangeSlider::lowerValueChanged, this, &MicroscopeManagerGUI::setVolumeScaleMin);
    connect(volumeScale, &RangeSlider::upperValueChanged, this, &MicroscopeManagerGUI::setVolumeScaleMax);
    connect(ui.volumeMin, &QDoubleSpinBox::valueChanged, this, &MicroscopeManagerGUI::setVolumeScaleSliderMin);
    connect(ui.volumeMax, &QDoubleSpinBox::valueChanged, this, &MicroscopeManagerGUI::setVolumeScaleSliderMax);
    connect(ui.framesPerVolumeSpinBox, &QSpinBox::valueChanged, this, &MicroscopeManagerGUI::setFramesPerVolume);
    connect(ui.volumesPerSecondSpinBox, &QSpinBox::valueChanged, this, &MicroscopeManagerGUI::setVolumesPerSecond);
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
    cfg.ReadConfigFile(configFile, mm, &ui);
    cfg.GetExperimentSettings(&volumeScaleMin, &volumeScaleMax, &framesPerVolume, &volumesPerSecond, &experimentSettingsDevice);
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
    img = QImage(buf, 1920, 1080, 1920, QImage::Format_Grayscale8, NULL, NULL);
    ui.imageDisplay->setPixmap(QPixmap::fromImage(img));

    mm->WriteFile(buf, 1920 * 1080);
    mm->CloseFile();
}

void MicroscopeManagerGUI::acquireHelper()
{
    if (ui.acquireButton->isChecked())
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
    if (!acquiring)
    {
        if (filepath == "")
        {
            setFilename();
        }

        ui.acquireButton->setText("Stop Acquisition");

        mm->SetFilename(filepath);
        mm->CreateFile();

        mm->StartAcquisition(GENTL_INFINITE);
        cameraThd = new AcquisitionDisplayThread(GENTL_INFINITE, mm, ui.imageDisplay, targetFrameInfo);
        acquiring = true;
    }
}

void MicroscopeManagerGUI::acquireStop()
{
    if (cameraThd)
    {
        cameraThd->StopThread();
        cameraThd->WaitForThread();
        delete cameraThd;
        cameraThd = NULL;

        acquiring = false;
    }

    mm->StopAcquisition();
    mm->CloseFile();

    ui.acquireButton->setText("Start Acquisition");
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


void MicroscopeManagerGUI::writeExperimentParameter(char parameter, std::string data)
{
    data = parameter + data + '\r';
    unsigned long long writeSize = data.size();

    mm->SerialWrite(experimentSettingsDevice, data.c_str(), writeSize);
}
