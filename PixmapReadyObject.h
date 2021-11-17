#pragma once
#include <qobject.h>
class PixmapReadyObject :
    public QObject
{
    Q_OBJECT

public:
    void sendSignal(const QPixmap&, bool);

signals:
    void pixmapReady(const QPixmap&, bool);
};

