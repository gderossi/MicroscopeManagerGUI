#include "ConfigDialog.h"
#include "QtSerialConfigBox.h"
#include "StateConfigBox.h"
#include "OdorantConfigBox.h"
#include "ConfigManager.h"
#include <QtWidgets>

ConfigDialog::ConfigDialog(std::vector<std::string> imMng, std::vector<std::string> cmMng, std::vector<std::string> srMng, std::vector<std::string> ports, QWidget* parent)
	: QDialog(parent),
	ports_(ports)
{
	ui.setupUi(this);

	for (std::string i : imMng)
	{
		ui.imageManagerComboBox->addItem(i.c_str());
	}
	for (std::string c : cmMng)
	{
		ui.cameraManagerComboBox->addItem(c.c_str());
	}
	for (std::string s : srMng)
	{
		ui.serialManagerComboBox->addItem(s.c_str());
	}

	ui.stateScrollChild->setLayout(new QVBoxLayout(ui.stateScrollChild));
	ui.serialScrollChild->setLayout(new QVBoxLayout(ui.serialScrollChild));
	ui.odorantScrollChild->setLayout(new QVBoxLayout(ui.odorantScrollChild));

	ui.stateScrollParent->setWidgetResizable(true);
	ui.stateScrollParent->setWidget(ui.stateScrollChild);

	ui.serialScrollParent->setWidgetResizable(true);
	ui.serialScrollParent->setWidget(ui.serialScrollChild);

	connect(ui.addSerialDevice, &QToolButton::clicked, this, &ConfigDialog::newSerial);
	connect(ui.addState, &QToolButton::clicked, this, &ConfigDialog::newState);
	connect(ui.addOdorant, &QToolButton::clicked, this, &ConfigDialog::newOdorant);

	connect(ui.configButtonBox->button(QDialogButtonBox::StandardButton::Ok), &QPushButton::clicked, this, &ConfigDialog::writeConfig);
	connect(ui.configButtonBox->button(QDialogButtonBox::StandardButton::Cancel), &QPushButton::clicked, this, &ConfigDialog::close);
}

ConfigDialog::~ConfigDialog()
{
}

void ConfigDialog::writeConfig()
{
	std::string filename = QFileDialog::getSaveFileName().toUtf8().constData();

	if (filename != "")
	{
		ConfigManager cfg;
		cfg.WriteConfigFile(filename, &ui);

		close();
	}
}

void ConfigDialog::newSerial()
{
	QtSerialConfigBox* s = new QtSerialConfigBox(ports_, ui.serialScrollChild);
	ui.serialScrollChild->layout()->addWidget(s);
	s->show();
}

void ConfigDialog::newState()
{
	StateConfigBox* s = new StateConfigBox(ui.stateScrollChild);
	ui.stateScrollChild->layout()->addWidget(s);
	s->show();
}

void ConfigDialog::newOdorant()
{
	OdorantConfigBox* o = new OdorantConfigBox(ui.odorantScrollChild);
	ui.odorantScrollChild->layout()->addWidget(o);
	o->show();
}
