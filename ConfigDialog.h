#pragma once

#include <QDialog>
#include "ui_ConfigDialog.h"
#include <vector>
#include <string>

class ConfigDialog : public QDialog
{
	Q_OBJECT

public:
	ConfigDialog(std::vector<std::string> imMng, std::vector<std::string> cmMng, std::vector<std::string> srMng, std::vector<std::string> ports, QWidget* parent = Q_NULLPTR);
	~ConfigDialog();

private slots:
	void writeConfig();
	void newSerial();
	void newState();
	void newOdorant();

private:
	Ui::ConfigDialog ui;
	std::vector<std::string> ports_;
};
