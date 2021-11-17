#include "PixmapReadyObject.h"

void PixmapReadyObject::sendSignal(const QPixmap& pixmap, bool acq)
{
	emit pixmapReady(pixmap, acq);
}
