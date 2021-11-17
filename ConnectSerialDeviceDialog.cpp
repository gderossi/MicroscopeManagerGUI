#include "ConnectSerialDeviceDialog.h"


ConnectSerialDeviceDialog::ConnectSerialDeviceDialog(std::vector<std::string> ports, MicroscopeManager* mm, QComboBox* deviceList, QWidget *parent)
	: QDialog(parent),
	mm_(mm),
	deviceList_(deviceList)
{
	ui.setupUi(this);

	for(std::string p : ports)
	{
		ui.portComboBox->addItem(p.c_str());
	}

	connect(ui.connectSerialConfirm->button(QDialogButtonBox::StandardButton::Ok), &QPushButton::clicked, this, &ConnectSerialDeviceDialog::CreateDevice);
	connect(ui.connectSerialConfirm->button(QDialogButtonBox::StandardButton::Cancel), &QPushButton::clicked, this, &ConnectSerialDeviceDialog::close);
}

ConnectSerialDeviceDialog::~ConnectSerialDeviceDialog()
{
}

void ConnectSerialDeviceDialog::CreateDevice()
{
	if (ui.deviceNameLineEdit->text() != "")
	{
		std::vector<std::string> exitCommands;
		for (QString command : ui.exitCommandsTextEdit->toPlainText().split('\n'))
		{
			exitCommands.push_back(command.toStdString());
		}

		std::string deviceName = ui.deviceNameLineEdit->text().toUtf8().constData();
		std::string portName = ui.portComboBox->currentText().toUtf8().constData();
		int baudrate = atoi(ui.baudrateComboBox->currentText().toUtf8().constData());

		mm_->ConnectSerialDevice(deviceName, portName, baudrate, exitCommands);

		deviceList_->addItem(deviceName.c_str());

		//serialThds.emplace(deviceName, new SerialConsoleThread(deviceName, mm, ui.consoleOutput));

		close();
	}
}
