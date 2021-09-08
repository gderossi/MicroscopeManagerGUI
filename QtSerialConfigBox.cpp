#include "QtSerialConfigBox.h"

QtSerialConfigBox::QtSerialConfigBox(std::vector<std::string> ports, QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	for (std::string p : ports)
	{
		ui.portComboBox->addItem(p.c_str());
	}

	setAttribute(Qt::WA_DeleteOnClose);
	connect(ui.deleteSerialDevice, &QToolButton::clicked, this, &QtSerialConfigBox::close);
}

QtSerialConfigBox::~QtSerialConfigBox()
{
}

Ui::QtSerialConfigBox* QtSerialConfigBox::getUi()
{
	return &ui;
}
