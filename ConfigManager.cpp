#include "ConfigManager.h"
#include "QtSerialConfigBox.h"
#include "OdorantConfigBox.h"
#include "StateConfigBox.h"
#include <fstream>

ConfigManager::ConfigManager() :
	volumeScaleMin(-1),
	volumeScaleMax(-1),
	framesPerVolume(-1),
	volumesPerSecond(-1)
{}

void ConfigManager::WriteConfigFile(std::string filename, Ui::ConfigDialog* cfg)
{
	std::string line;
	std::ofstream file(filename);

	if (file.is_open())
	{
		file << "MANAGER_SETTINGS\n";
		line = cfg->imageManagerComboBox->currentText().toUtf8().constData();
		line += '\n';
		file << line;
		line = cfg->cameraManagerComboBox->currentText().toUtf8().constData();
		line += '\n';
		file << line;
		line = cfg->serialManagerComboBox->currentText().toUtf8().constData();
		line += '\n';
		file << line;
		file << '\n';
		
		file << "CAMERA_SETTINGS\n";
		line = "Integer:Width=" + std::to_string(cfg->widthSpinBox->value());
		line += '\n';
		file << line;
		line = "Integer:Height=" + std::to_string(cfg->heightSpinBox->value());
		line += '\n';
		file << line;
		line = cfg->pixelFormatComboBox->currentText().toUtf8().constData();
		line = "Enumeration:PixelFormat=" + line + '\n';
		file << line;
		line = cfg->acquisitionModeComboBox->currentText().toUtf8().constData();
		line = "Enumeration:AcquisitionMode=" + line + '\n';
		file << line;
		line = std::to_string(cfg->frameRateDoubleSpinBox->value());
		line = "Float:AcquisitionFrameRate=" + line + '\n';
		file << line;
		line = std::to_string(cfg->exposureTimeDoubleSpinBox->value());
		line = "Float:ExposureTime=" + line + '\n';
		file << line;
		file << '\n';

		file << "EXPERIMENT_SETTINGS\n";
		bool experimentSettingsDevice = false;
		for (int i = 1; i < cfg->serialScrollChild->children().size(); i++)
		{
			QtSerialConfigBox* s = (QtSerialConfigBox*)cfg->serialScrollChild->children()[i];
			Ui::QtSerialConfigBox* ui = s->getUi();

			line = ui->startupCommandsTextEdit->toPlainText().toUtf8().constData();
			if (line == "EXPERIMENT_SETTINGS")
			{
				line = ui->deviceNameLineEdit->text().toUtf8().constData();
				line += '\n';
				file << line;
				experimentSettingsDevice = true;
				break;
			}
		}
		if (!experimentSettingsDevice)
		{
			line = "\n";
			file << line;
		}
		line = std::to_string(cfg->framesPerVolumeSpinBox->value());
		line += "\n";
		file << line;
		line = std::to_string(cfg->volumesPerSecondSpinBox->value());
		line += '\n';
		file << line;
		line = std::to_string(cfg->volumeRangeMinDoubleSpinBox->value());
		line += '\n';
		file << line;
		line = std::to_string(cfg->volumeRangeMaxDoubleSpinBox->value());
		line += '\n';
		file << line;
		line = std::to_string(cfg->laserModeComboBox->currentIndex());
		line += '\n';
		file << line;
		line = std::to_string(cfg->resonantScannerRangeDoubleSpinBox->value());
		line += "\n\n";
		file << line;

		file << "ODORANT_SETTINGS\n";
		if (cfg->odorantScrollChild->children().size() > 1)
		{
			for (int i = 1; i < cfg->odorantScrollChild->children().size(); i++)
			{
				OdorantConfigBox* o = (OdorantConfigBox*)cfg->odorantScrollChild->children()[i];
				Ui::OdorantConfigBox* odorantUi = o->getUi();

				line = std::to_string(odorantUi->odorantComboBox->currentData().toInt());
				line += '\n';
				file << line;
			}
		}
		line = "\n";
		file << line;

		file << "STATE_SETTINGS\n";
		if (cfg->stateScrollChild->children().size() > 1)
		{
			for (int i = 1; i < cfg->stateScrollChild->children().size(); i++)
			{
				StateConfigBox* st = (StateConfigBox*)cfg->stateScrollChild->children()[i];
				Ui::StateConfigBox* stateUi = st->getUi();

				line = std::to_string(stateUi->stateComboBox->currentData().toInt());
				line += '\n';
				file << line;
				line = std::to_string(stateUi->durationSpinBox->value());
				line += '\n';
				file << line;
			}
		}
		line = "\n";
		file << line;

		if (!cfg->serialScrollChild->children().isEmpty())
		{
			for (int i = 1; i < cfg->serialScrollChild->children().size(); i++)
			{
				QtSerialConfigBox* s = (QtSerialConfigBox*)cfg->serialScrollChild->children()[i];
				Ui::QtSerialConfigBox* ui = s->getUi();

				file << "SERIAL_DEVICE\n";
				line = ui->deviceNameLineEdit->text().toUtf8().constData();
				line += '\n';
				file << line;
				line = ui->portComboBox->currentText().toUtf8().constData();
				line += '\n';
				file << line;
				line = ui->baudrateComboBox->currentText().toUtf8().constData();
				line += '\n';
				file << line;
				line = ui->startupCommandsTextEdit->toPlainText().toUtf8().constData();
				if (line != "EXPERIMENT_SETTINGS")
				{
					line += '\n';
					file << line;
					file << "EXIT_COMMANDS\n";
				}
				else
				{
					/*line = std::to_string(cfg->framesPerVolumeSpinBox->value());
					line = SLICES_PER_VOLUME + line + '\n';
					file << line;
					line = std::to_string(cfg->volumesPerSecondSpinBox->value());
					line = VOLUMES_PER_SECOND + line + '\n';
					file << line;
					line = std::to_string(cfg->volumeRangeMinDoubleSpinBox->value());
					line = VOLUME_SCALE_MIN + line + '\n';
					file << line;
					line = std::to_string(cfg->volumeRangeMaxDoubleSpinBox->value());
					line = VOLUME_SCALE_MAX + line + '\n';
					file << line;

					if (cfg->odorantScrollChild->children().size() > 1)
					{
						line = ODORANT_ORDER;

						for (int i = 1; i < cfg->odorantScrollChild->children().size(); i++)
						{
							OdorantConfigBox* o = (OdorantConfigBox*)cfg->odorantScrollChild->children()[i];
							Ui::OdorantConfigBox* odorantUi = o->getUi();

							line += std::to_string(odorantUi->odorantComboBox->currentData().toInt());
						}

						line += '\n';
						file << line;
					}

					if (cfg->stateScrollChild->children().size() > 1)
					{
						line = STATE_ORDER + std::to_string(cfg->stateScrollChild->children().size() - 1);
						line += '\n';
						file << line;

						for (int i = 1; i < cfg->stateScrollChild->children().size(); i++)
						{
							StateConfigBox* st = (StateConfigBox*)cfg->stateScrollChild->children()[i];
							Ui::StateConfigBox* stateUi = st->getUi();

							line = std::to_string(stateUi->stateComboBox->currentData().toInt() + stateUi->durationSpinBox->value());
							line += '\n';
							file << line;
						}
					}*/

					file << "EXIT_COMMANDS\n";
				}

				line = ui->exitCommandsTextEdit->toPlainText().toUtf8().constData();
				line += '\n';
				file << line;
				file << "END_SERIAL_DEVICE\n\n";
			}
		}

		file.close();
	}
}

void ConfigManager::ReadConfigFile(std::string filename, MicroscopeManager* mm, Ui::MicroscopeManagerGUIClass* ui)
{
	std::string line;
	std::ifstream file(filename);

	int substringStart = 0;
	int substringEnd = 0;

	std::string managerType;

	std::string cameraSettingType;
	std::string cameraSettingName;
	long long intValue = 0;
	double floatValue = 0;
	std::string stringValue;
	
	std::string serialDeviceName;
	std::string serialDevicePort;
	int baudrate = 0;
	std::string serialCommand;

	if (file.is_open())
	{
		while (getline(file, line))
		{
			//Manager config
			if (line == "MANAGER_SETTINGS")
			{
				getline(file, line);
				mm->CreateImageManager(line, NULL);

				getline(file, line);
				mm->CreateCameraManager(line, NULL);

				getline(file, line);
				mm->CreateSerialManager(line, NULL);
			}

			//Camera config
			if (line == "CAMERA_SETTINGS")
			{
				while (getline(file, line))
				{
					if (line == "")
					{
						break;
					}

					substringStart = 0;
					substringEnd = line.find(":");
					cameraSettingType = line.substr(substringStart, substringEnd - substringStart);

					substringStart = substringEnd + 1;
					substringEnd = line.find('=');
					cameraSettingName = line.substr(substringStart, substringEnd - substringStart);

					substringStart = substringEnd + 1;
					substringEnd = line.size();
					stringValue = line.substr(substringStart, substringEnd - substringStart);

					if (cameraSettingType == "Integer")
					{
						intValue = atoi(stringValue.c_str());
						mm->SetCameraIntParameter(REMOTE_MODULE, cameraSettingName, intValue);
					}
					else if (cameraSettingType == "Float")
					{
						floatValue = atof(stringValue.c_str());
						mm->SetCameraFloatParameter(REMOTE_MODULE, cameraSettingName, floatValue);
					}
					else if (cameraSettingType == "Enumeration")
					{
						mm->SetCameraStringParameter(REMOTE_MODULE, cameraSettingName, stringValue);
					}
				}
			}

			//Experiment settings
			if (line == "EXPERIMENT_SETTINGS")
			{
				getline(file, line);
				experimentDevice = line;
				getline(file, line);
				framesPerVolume = atoi(line.c_str());
				getline(file, line);
				volumesPerSecond = atoi(line.c_str());
				getline(file, line);
				volumeScaleMin = atof(line.c_str());
				getline(file, line);
				volumeScaleMax = atof(line.c_str());
				getline(file, line);
				laserMode = atoi(line.c_str());
				getline(file, line);
				scannerAmplitude = atof(line.c_str());
			}

			if (line == "ODORANT_SETTINGS")
			{
				while (getline(file, line))
				{
					if (line == "")
					{
						break;
					}

					OdorantConfigBox* o = new OdorantConfigBox(ui->odorantOrderScrollAreaContents);
					ui->odorantOrderScrollAreaContents->layout()->addWidget(o);
					Ui::OdorantConfigBox* oUi = o->getUi();
					oUi->odorantComboBox->setCurrentIndex(atoi(line.c_str())-1);
					o->show();
				}
			}

			if (line == "STATE_SETTINGS")
			{
				while (getline(file, line))
				{
					if (line == "")
					{
						break;
					}

					StateConfigBox* s = new StateConfigBox(ui->stateOrderScrollAreaContents);
					ui->stateOrderScrollAreaContents->layout()->addWidget(s);
					Ui::StateConfigBox* sUi = s->getUi();
					sUi->stateComboBox->setCurrentIndex((atoi(line.c_str())/0x00010000)-1);
					getline(file, line);
					sUi->durationSpinBox->setValue(atoi(line.c_str()));
					s->show();
				}
			}

			//Serial config
			if (line == "SERIAL_DEVICE")
			{
				std::vector<std::string> startCommands;
				std::vector<std::string> exitCommands;
				getline(file, line);
				serialDeviceName = line;
				getline(file, line);
				serialDevicePort = line;
				getline(file, line);
				baudrate = atoi(line.c_str());

				while (getline(file, line))
				{
					if (line == "EXIT_COMMANDS")
					{
						break;
					}

					startCommands.push_back(line + '\r');
				}
				while (getline(file, line))
				{
					if (line == "END_SERIAL_DEVICE")
					{
						break;
					}

					exitCommands.push_back(line + '\r');
				}
				mm->ConnectSerialDevice(serialDeviceName, serialDevicePort, baudrate, exitCommands);
				ui->consoleDeviceList->addItem(serialDeviceName.c_str());
				
				for (std::string command : startCommands)
				{
					mm->SerialWrite(serialDeviceName, command.c_str(), command.size());
				}
			}
		}

		file.close();
	}
}

void ConfigManager::GetExperimentSettings(double* vsMin, double* vsMax, int* fpv, int* vps, int* lm, double* sa, std::string* exp)
{
	if (volumeScaleMin != -1)
	{
		*vsMin = volumeScaleMin;
	}
	if (volumeScaleMax != -1)
	{
		*vsMax = volumeScaleMax;
	}
	if (framesPerVolume != -1)
	{
		*fpv = framesPerVolume;
	}
	if (volumesPerSecond != -1)
	{
		*vps = volumesPerSecond;
	}
	if (laserMode != -1)
	{
		*lm = laserMode;
	}
	if (scannerAmplitude != -1)
	{
		*sa = scannerAmplitude;
	}
	if (experimentDevice != "")
	{
		*exp = experimentDevice;
	}
}