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

	adjustParentHeight(0);
}

QtSerialConfigBox::~QtSerialConfigBox()
{
	adjustParentHeight(1);
}

Ui::QtSerialConfigBox* QtSerialConfigBox::getUi()
{
	return &ui;
}

void QtSerialConfigBox::adjustParentHeight(int offset)
{
	try
	{
		QWidget* parent = parentWidget();
		QObjectList children = parent->children();
		int contentHeight = 0;

		if (children.size() > 1)
		{
			for (int i = 1; i < children.size() - offset; i++)
			{
				QWidget* child = (QWidget*)children[i];
				if (child)
				{
					contentHeight += child->height() + 10;
				}
			}

			parent->setFixedHeight(contentHeight);
		}
	}
	catch(...)
	{ }
}
