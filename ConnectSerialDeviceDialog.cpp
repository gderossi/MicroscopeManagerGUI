#include "ConnectSerialDeviceDialog.h"


ConnectSerialDeviceDialog::ConnectSerialDeviceDialog(std::vector<std::string> ports, QObject* mainWindow, QWidget *parent)
	: QDialog(parent),
	mainWindow_(mainWindow)
{
	ui.setupUi(this);

	for(std::string p : ports)
	{
		ui.portComboBox->addItem(p.c_str());
	}

	connect(ui.connectSerialConfirm->button(QDialogButtonBox::StandardButton::Ok), &QPushButton::clicked, this, &ConnectSerialDeviceDialog::CreateDevice);
	connect(ui.connectSerialConfirm->button(QDialogButtonBox::StandardButton::Cancel), &QPushButton::clicked, this, &ConnectSerialDeviceDialog::close);
	connect(this, SIGNAL(serialDeviceReady(std::string, std::string, int, std::vector<std::string>)), mainWindow_, SLOT(connectSerialDevice(std::string, std::string, int, std::vector<std::string>)));
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

		emit serialDeviceReady(deviceName, portName, baudrate, exitCommands);

		close();
	}
}
