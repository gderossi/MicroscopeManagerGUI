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

	adjustParentHeight(0);
}

OdorantConfigBox::~OdorantConfigBox()
{
	adjustParentHeight(1);
}

Ui::OdorantConfigBox* OdorantConfigBox::getUi()
{
	return &ui;
}

void OdorantConfigBox::adjustParentHeight(int offset)
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
	{}
}
