#pragma once
#include <qobject.h>
#include <queue>
#include <mutex>

class SerialQueueObject :
    public QObject
{
    Q_OBJECT

public:
    SerialQueueObject(QObject* mainWindow);
    void sendSignal(std::string);
    std::mutex qMutex;

signals:
    void serialMessage(std::string);
};

