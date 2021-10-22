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

	adjustParentHeight(0);
}

StateConfigBox::~StateConfigBox()
{
	adjustParentHeight(1);
}

Ui::StateConfigBox* StateConfigBox::getUi()
{
	return &ui;
}

void StateConfigBox::adjustParentHeight(int offset)
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
