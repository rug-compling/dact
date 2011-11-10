#ifndef DELAYEDLOADFILECALLBACK_H
#define DELAYEDLOADFILECALLBACK_H

#include <QObject>
#include <QString>

class DactMacrosModel;

class DelayedLoadFileCallback : public QObject
{
    Q_OBJECT

public:
    DelayedLoadFileCallback(DactMacrosModel *model, QString const &fileName, QObject *parent = 0);

public slots:
    void invokeOnce();

private:
    DactMacrosModel *d_model;
    QString d_fileName;
};

#endif

