#pragma once

#include <QWidget>
#include "ui_QtSerialConfigBox.h"

class QtSerialConfigBox : public QWidget
{
	Q_OBJECT

public:
	QtSerialConfigBox(std::vector<std::string> ports, QWidget* parent = Q_NULLPTR);
	~QtSerialConfigBox();
	Ui::QtSerialConfigBox* getUi();

private:
	void adjustParentHeight(int offset);
	Ui::QtSerialConfigBox ui;
};
