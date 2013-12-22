#ifndef SUBSET_MODEL_HH
#define SUBSET_MODEL_HH

#include <QAbstractTableModel>
#include <QModelIndex>
#include <QSharedPointer>
#include <QVariant>

class SubSetModel : public QAbstractTableModel
{
  Q_OBJECT
public:
    SubSetModel(QSharedPointer<QAbstractTableModel> underlyingModel,
        size_t size = 100, size_t start = 0);

    virtual int columnCount(QModelIndex const &parent = QModelIndex()) const;
    virtual QVariant data(QModelIndex const &index, int role = Qt::DisplayRole) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
        int role = Qt::DisplayRole) const;
    virtual QModelIndex index(int row, int column,
        QModelIndex const &parent = QModelIndex()) const;
    virtual int rowCount(QModelIndex const &parent = QModelIndex()) const;
private:
    QSharedPointer<QAbstractTableModel> d_model;
    size_t d_size;
    size_t d_start;
private slots:
    void modelDataChanged(QModelIndex const &topLeft, QModelIndex const &bottomRight);
signals:
    void dataChanged(QModelIndex const &topleft, QModelIndex const &bottomright);
};

#endif // SUBSET_MODEL_HH
