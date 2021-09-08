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
		for (int i = 1; i < cfg->serialScrollChild->children().size(); i++)
		{
			QtSerialConfigBox* s = (QtSerialConfigBox*)cfg->serialScrollChild->children()[i];
			Ui::QtSerialConfigBox* ui = s->getUi();

			line = ui->plainTextEdit->toPlainText().toUtf8().constData();
			if (line == "EXPERIMENT_SETTINGS")
			{
				line = ui->deviceNameLineEdit->text().toUtf8().constData();
				line += '\n';
				file << line;
				break;
			}
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
		line += "\n\n";
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
				line = ui->plainTextEdit->toPlainText().toUtf8().constData();
				if (line == "EXPERIMENT_SETTINGS")
				{
					line = std::to_string(cfg->framesPerVolumeSpinBox->value());
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
					}

					file << '\n';
				}
				else
				{
					line += '\n';
					file << line;
					file << '\n';
				}
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
	unsigned long long writeSize = 0;

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
			}

			//Serial config
			if (line == "SERIAL_DEVICE")
			{
				getline(file, line);
				serialDeviceName = line;
				getline(file, line);
				serialDevicePort = line;
				getline(file, line);
				baudrate = atoi(line.c_str());
				mm->ConnectSerialDevice(serialDeviceName, serialDevicePort, baudrate);

				ui->consoleDeviceList->addItem(serialDeviceName.c_str());

				while (getline(file, line))
				{
					if (line == "")
					{
						break;
					}

					serialCommand = line;
					serialCommand += '\r';
					writeSize = serialCommand.size();
					mm->SerialWrite(serialDeviceName, serialCommand.c_str(), writeSize);
				}
			}
		}

		file.close();
	}
}

void ConfigManager::GetExperimentSettings(double* vsMin, double* vsMax, int* fpv, int* vps, std::string* exp)
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
	if (experimentDevice != "")
	{
		*exp = experimentDevice;
	}
}