/* vim: set sw=4 sts=4 ft=cpp et foldmethod=syntax : */

/*
 * Copyright (c) 2013 Tiziano Müller <tm@dev-zero.ch>
 *
 *
 *
 */

#ifndef SPACE_USAGE_MODEL_HH
#define SPACE_USAGE_MODEL_HH

#include <tuple>
#include <QtCore/QAbstractTableModel>

class SpaceUsageModel :
    public QAbstractTableModel
{
    Q_OBJECT
public:
    enum Entries { BACKUP = 0, INCREMENTAL = 1, FREE = 2, QUOTA = 3 };

     /** Order of elements in the tuple: backup, incremental, free, quota
     */
    typedef std::tuple<double, double, double, double> SpaceUsageData;

    /** Construct a model with all space usage values set to 0
     */
    SpaceUsageModel(QObject* p = nullptr);

    /** Set the space usage data using the custom type.
     */
    void setSpaceUsage(const SpaceUsageData& spaceUsage);

    /** Set the space usage data using separate values.
     */
    void setSpaceUsage(double backup, double incremental, double free, double quota);

    /** Get the space usage tuple
     */
    SpaceUsageData getSpaceUsage() const;

    /** \sa http://qt-project.org/doc/qt-4.8/qabstracttablemodel.html
     */
    int rowCount(const QModelIndex& parent = QModelIndex()) const;

    /** \sa http://qt-project.org/doc/qt-4.8/qabstracttablemodel.html
     */
    int columnCount(const QModelIndex& parent = QModelIndex()) const;

    /** \sa http://qt-project.org/doc/qt-4.8/qabstracttablemodel.html
     */
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;

    /** \sa http://qt-project.org/doc/qt-4.8/qabstracttablemodel.html
     */
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);

    /** \sa http://qt-project.org/doc/qt-4.8/qabstracttablemodel.html
     */
    Qt::ItemFlags flags(const QModelIndex & index) const;

private:
    SpaceUsageData _spaceUsage;
};

#endif // SPACE_USAGE_MODEL_HH
