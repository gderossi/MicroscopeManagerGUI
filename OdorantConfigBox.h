#pragma once

#include <QWidget>
#include "ui_OdorantConfigBox.h"

class OdorantConfigBox : public QWidget
{
	Q_OBJECT

public:
	OdorantConfigBox(QWidget *parent = Q_NULLPTR);
	~OdorantConfigBox();
	Ui::OdorantConfigBox* getUi();

private:
	void adjustParentHeight(int offset);
	Ui::OdorantConfigBox ui;
};
