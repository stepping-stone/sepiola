/* vim: set sw=4 sts=4 ft=cpp et foldmethod=syntax : */

/*
 * Copyright (c) 2011 Tiziano Müller <tm@dev-zero.ch>
 *
 *
 *
 */

#include "space_usage_model.hh"

#include <QtGui/QColor>

SpaceUsageModel::SpaceUsageModel(QObject* p = 0) :
    QAbstractTableModel(p),
    _spaceUsage(0, 0, 0, 0)
{
}

int SpaceUsageModel::rowCount(const QModelIndex&) const
{
    return std::tuple_size<SpaceUsageData>::value;
}

int SpaceUsageModel::columnCount(const QModelIndex&) const
{
    return 1;
}

QVariant SpaceUsageModel::data(const QModelIndex & index, int role) const
{
    if (!index.isValid() || (index.column() != 0))
        return QVariant();

    if (role == Qt::DisplayRole)
    {
        switch (index.row())
        {
            case 0: return std::get<0>(_spaceUsage);
            case 1: return std::get<1>(_spaceUsage);
            case 2: return std::get<2>(_spaceUsage);
            //case 3: return std::get<3>(_spaceUsage);
        }
    } else if (role == Qt::DecorationRole)
    {
        switch (index.row())
        {
            case 0: return QColor(255, 0, 0, 255);
            case 1: return QColor(255, 175, 0, 255);
            case 2: return QColor(0, 191, 0, 255);
            //case 3: return QColor(0, 0, 0, 0);
        }
    }

    return QVariant();
}

bool SpaceUsageModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if ((role != Qt::EditRole)
            || !index.isValid()
            || (index.column() != 0)
            || !value.canConvert<double>())
        return false;

    switch (index.row())
    {
        case 0: std::get<0>(_spaceUsage) = value.toDouble(); break;
        case 1: std::get<1>(_spaceUsage) = value.toDouble(); break;
        case 2: std::get<2>(_spaceUsage) = value.toDouble(); break;
        case 3: std::get<3>(_spaceUsage) = value.toDouble(); break;
        default: return false;
    }

    emit(dataChanged(index, index));

    return true;
}
    
void SpaceUsageModel::setSpaceUsage(const SpaceUsageData& spaceUsage)
{
    _spaceUsage = spaceUsage;
    emit(dataChanged(index(0, 0), index(std::tuple_size<SpaceUsageData>::value, 0)));
}
    
void SpaceUsageModel::setSpaceUsage(double backup, double incremental, double free, double quota)
{
    setSpaceUsage(std::tie(backup, incremental, free, quota));
}
    
SpaceUsageModel::SpaceUsageData SpaceUsageModel::getSpaceUsage() const
{
    return _spaceUsage;
}

Qt::ItemFlags SpaceUsageModel::flags(const QModelIndex & index) const
{
    if (!index.isValid()
            || (index.column() != 0)
            || (static_cast<size_t>(index.row()) >= std::tuple_size<SpaceUsageData>::value))
        return Qt::NoItemFlags;

    return (QAbstractTableModel::flags(index) | Qt::ItemIsEditable);
}
