#include <algorithm>

#include <QModelIndex>
#include <QSharedPointer>

#include <SubSetModel.hh>

SubSetModel::SubSetModel(QSharedPointer<QAbstractTableModel> underlyingModel,
        size_t size, size_t start)
    : d_model(underlyingModel), d_size(size), d_start(start)
{
    connect(d_model.data(),
        SIGNAL(dataChanged(QModelIndex const &, QModelIndex const &)),
        SLOT(modelDataChanged(QModelIndex const &, QModelIndex const &)));
}

int SubSetModel::columnCount(QModelIndex const &parent) const
{
    return d_model->columnCount(parent);
}

QVariant SubSetModel::data(QModelIndex const &index, int role) const
{
    return d_model->data(index, role);
}

QVariant SubSetModel::headerData(int section, Qt::Orientation orientation,
    int role) const
{
    return d_model->headerData(section, orientation, role);
}

QModelIndex SubSetModel::index(int row, int column, QModelIndex const &parent) const
{
  return d_model->index(row, column, parent);
}

void SubSetModel::modelDataChanged(QModelIndex const &topLeft, QModelIndex const &bottomRight)
{
    int startRow = topLeft.row();
    int endRow = bottomRight.row();
    
    if (startRow >= d_size)
      return;

    emit layoutAboutToBeChanged();
    emit dataChanged(topLeft,
        index(std::min(static_cast<int>(d_size - 1), endRow), bottomRight.column()));
    emit layoutChanged();
}

int SubSetModel::rowCount(QModelIndex const &parent) const
{
    int modelCount = d_model->rowCount(parent);
    return std::min(static_cast<int>(d_size), modelCount);
}
