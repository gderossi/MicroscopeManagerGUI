#pragma once

#include <QWidget>
#include "ui_StateConfigBox.h"

class StateConfigBox : public QWidget
{
	Q_OBJECT

public:
	StateConfigBox(QWidget *parent = Q_NULLPTR);
	~StateConfigBox();
	Ui::StateConfigBox* getUi();

private:
	Ui::StateConfigBox ui;
};
