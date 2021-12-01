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
	ConnectSerialDeviceDialog(std::vector<std::string> ports, QObject* mainWindow_, QWidget *parent = Q_NULLPTR);
	~ConnectSerialDeviceDialog();
	void CreateDevice();

signals:
	void serialDeviceReady(std::string, std::string, int, std::vector<std::string>);

private:
	Ui::ConnectSerialDeviceDialog ui;
	QObject* mainWindow_;
};
