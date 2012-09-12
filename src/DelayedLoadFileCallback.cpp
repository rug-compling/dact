#include <QMutex>

#include "DelayedLoadFileCallback.hh"
#include "DactMacrosModel.hh"

DelayedLoadFileCallback::DelayedLoadFileCallback(DactMacrosModel *model, QMutex *reloadMutex,
	QString const &fileName, QObject *parent)
:
    QObject(parent),
    d_model(model),
    d_reloadMutex(reloadMutex),
    d_fileName(fileName)
{}

void DelayedLoadFileCallback::invokeOnce()
{
    d_model->loadFile(d_fileName);
    d_reloadMutex->unlock();
    delete this; // suicide!
}
