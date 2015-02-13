/* vim: set sw=4 sts=4 ft=cpp et foldmethod=syntax : */

/*
 * Copyright (c) 2011 Tiziano Müller <tm@dev-zero.ch>
 *
 *
 *
 */

#ifndef STACKED_BAR_VIEW_HH
#define STACKED_BAR_VIEW_HH

#include <QtGui/QAbstractItemView>
#include <QtGui/QPixmap>

class StackedBarView :
    public QAbstractItemView
{
    Q_OBJECT

public:
    StackedBarView(QWidget* p = 0);

    QModelIndex	indexAt(const QPoint & point) const;

    void scrollTo(const QModelIndex & index, ScrollHint hint = EnsureVisible);

    QRect visualRect(const QModelIndex & index) const;

    QPixmap legendIcon(const QModelIndex & index) const;

protected:
    int	horizontalOffset() const;
    int	verticalOffset() const;

    bool isIndexHidden(const QModelIndex & index) const;

    QModelIndex	moveCursor(CursorAction cursorAction, Qt::KeyboardModifiers modifiers);

    void setSelection(const QRect & rect, QItemSelectionModel::SelectionFlags flags);

    QRegion	visualRegionForSelection(const QItemSelection & selection) const;

    void paintEvent(QPaintEvent* event);

    bool edit (const QModelIndex&, EditTrigger, QEvent*);

protected slots:
    void dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void rowsInserted(const QModelIndex &parent, int start, int end);
    void rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end);

private:
    void _updateColorValues();

    int _margin, _stackbarHeight;
    double _totalValue;
    size_t _validItems;
};

#endif
