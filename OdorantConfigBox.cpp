#include "OdorantConfigBox.h"

OdorantConfigBox::OdorantConfigBox(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	
	for (int i = 0; i < ui.odorantComboBox->count(); i++)
	{
		ui.odorantComboBox->setItemData(i, i + 1);
	}

	setAttribute(Qt::WA_DeleteOnClose);
	connect(ui.deleteOdorant, &QToolButton::clicked, this, &OdorantConfigBox::close);
}

OdorantConfigBox::~OdorantConfigBox()
{
}

Ui::OdorantConfigBox* OdorantConfigBox::getUi()
{
	return &ui;
}
