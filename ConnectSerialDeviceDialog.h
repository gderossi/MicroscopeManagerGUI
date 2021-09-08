#pragma once

#include <QDialog>
#include "ui_ConnectSerialDeviceDialog.h"
#include <string>
#include <vector>
#include "MicroscopeManager.h"
#include <QtWidgets>

class ConnectSerialDeviceDialog : public QDialog
{
	Q_OBJECT

public:
	ConnectSerialDeviceDialog(std::vector<std::string> ports, MicroscopeManager* mm, QComboBox* deviceList, QWidget *parent = Q_NULLPTR);
	~ConnectSerialDeviceDialog();
	void CreateDevice();

private:
	Ui::ConnectSerialDeviceDialog ui;
	MicroscopeManager* mm_;
	QComboBox* deviceList_;
};
