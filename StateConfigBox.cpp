#include "StateConfigBox.h"

StateConfigBox::StateConfigBox(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	ui.stateComboBox->setItemData(0, 0x00010000);
	ui.stateComboBox->setItemData(1, 0x00020000);
	ui.stateComboBox->setItemData(2, 0x00030000);

	setAttribute(Qt::WA_DeleteOnClose);
	connect(ui.deleteState, &QToolButton::clicked, this, &StateConfigBox::close);
}

StateConfigBox::~StateConfigBox()
{
}

Ui::StateConfigBox* StateConfigBox::getUi()
{
	return &ui;
}
