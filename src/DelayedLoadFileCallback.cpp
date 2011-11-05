#include "DelayedLoadFileCallback.hh"
#include "DactMacrosModel.hh"

DelayedLoadFileCallback::DelayedLoadFileCallback(DactMacrosModel *model, QString const &fileName, QObject *parent)
:
    QObject(parent),
    d_model(model),
    d_fileName(fileName)
{}

void DelayedLoadFileCallback::invokeOnce()
{
    d_model->loadFile(d_fileName);
    delete this; // suicide!
}
