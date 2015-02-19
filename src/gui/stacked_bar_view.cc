/* vim: set sw=4 sts=4 ft=cpp et foldmethod=syntax : */

/*
 * Copyright (c) 2011 Tiziano Müller <tm@dev-zero.ch>
 *
 *
 *
 */

#include "stacked_bar_view.hh"

#include <QtGui/QPen>
#include <QtGui/QPainter>
#include <QtGui/QPaintEvent>
#include <QtGui/QScrollBar>

namespace
{
    static const int DATA_COLUMN = 0;
    static const int COLOR_COLUMN = 0;
}

StackedBarView::StackedBarView(QWidget* p) :
    QAbstractItemView(p),
    _margin(0),
    _stackbarHeight(16),
    _totalValue(0.0),
    _validItems(0)
{
}

QModelIndex	StackedBarView::indexAt(const QPoint& point) const
{
    if (_validItems == 0)
        return QModelIndex();

    // Transform the view coordinates into contents widget coordinates.
    int wx = point.x() + horizontalScrollBar()->value() - _margin;
    int wy = point.y() + verticalScrollBar()->value() - _margin;

    if ((wx < 0) || (wx > size().width()-_margin) || (wy < 0) || (wy > size().width()-_margin))
        return QModelIndex();

    double start(0.);

    for (int row(start), end(model()->rowCount(rootIndex())); row <= end; ++row)
    {
        const QModelIndex valueIndex(model()->index(row, DATA_COLUMN, rootIndex()));
        const double value(model()->data(valueIndex).toDouble());
        const QString color(model()->data(model()->index(row, COLOR_COLUMN, rootIndex()), Qt::DecorationRole).toString());

        if (value > 0.0 && !color.isNull())
        {
            start += (size().width()-2.*_margin) * (value/_totalValue);
            if (point.x() < start)
                return valueIndex;
        }
    }

    return QModelIndex();
}

void StackedBarView::scrollTo(const QModelIndex&, ScrollHint)
{
}

QRect StackedBarView::visualRect(const QModelIndex&) const
{
    return QRect();
}

int	StackedBarView::horizontalOffset() const
{
    return horizontalScrollBar()->value();
}

int	StackedBarView::verticalOffset() const
{
    return verticalScrollBar()->value();
}

bool StackedBarView::isIndexHidden(const QModelIndex& index) const
{
    const QString color(model()->data(model()->index(index.row(), COLOR_COLUMN, rootIndex()), Qt::DecorationRole).toString());
    const double value(model()->data(model()->index(index.row(), DATA_COLUMN, rootIndex())).toDouble());
    return (value <= 0.0 || color.isNull());
}

QModelIndex	StackedBarView::moveCursor(CursorAction, Qt::KeyboardModifiers)
{
    return QModelIndex();
}

void StackedBarView::setSelection(const QRect &, QItemSelectionModel::SelectionFlags)
{
}

QRegion	StackedBarView::visualRegionForSelection(const QItemSelection &) const
{
    return QRegion();
}

void StackedBarView::paintEvent(QPaintEvent* event)
{
    QStyleOptionViewItem option(viewOptions());

    QBrush background(option.palette.color(QPalette::Window));
    QPen foreground(option.palette.color(QPalette::WindowText));

    QPainter painter(viewport());
    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(event->rect(), background);
    painter.setPen(foreground);

    painter.save();

    const double stackbarTotalLength(size().width()-2.*_margin);
    const QRect barRect(_margin, _margin, stackbarTotalLength, _stackbarHeight);

    // Viewport rectangles
    painter.translate(barRect.x() - horizontalScrollBar()->value(),
                      barRect.y() - verticalScrollBar()->value());
    painter.drawRect(0, 0, stackbarTotalLength, _stackbarHeight);
    painter.setPen(Qt::NoPen);

    QLinearGradient gradient( QPoint(0, 0), QPoint(0, _stackbarHeight));
    gradient.setColorAt(0, Qt::white);
    gradient.setColorAt(1, Qt::black);

    if (_validItems > 0)
    {
        double start(0.);
        QAbstractItemModel * m(model());

        for (int row(0), end(m->rowCount(rootIndex())); row <= end; ++row)
        {
            const double value(m->data(m->index(row, DATA_COLUMN, rootIndex())).toDouble());
            const QString color(m->data(m->index(row, COLOR_COLUMN, rootIndex()), Qt::DecorationRole).toString());

            if (value > 0.0 && !color.isNull())
            {
                const double length(stackbarTotalLength * (value/_totalValue));
                gradient.setColorAt(0.5, color);
                painter.setBrush(gradient);
                painter.drawRect(QRectF(start, 0., length, _stackbarHeight));
                start += length;
            }
        }
    }
    else
    {
        // draw a black background bar
        gradient.setColorAt(0.5, Qt::black);
        painter.setBrush(gradient);
        painter.drawRect(QRectF(0., 0., stackbarTotalLength, _stackbarHeight));
    }

    painter.restore();
}

void StackedBarView::dataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight)
{
    QAbstractItemView::dataChanged(topLeft, bottomRight);

    _validItems = 0;
    _totalValue = 0.0;

    QAbstractItemModel * m(model());

    for (int row(0), end(m->rowCount(rootIndex())); row <= end; ++row)
    {
        double value(m->data(m->index(row, DATA_COLUMN, rootIndex())).toDouble());
        QString color(m->data(m->index(row, COLOR_COLUMN, rootIndex()), Qt::DecorationRole).toString());

        if (value > 0.0 && !color.isNull())
        {
            _validItems += 1;
            _totalValue += value;
        }    
    }

    viewport()->update();
}

void StackedBarView::rowsInserted(const QModelIndex &parent, int start, int end)
{
    QAbstractItemModel * m(model());

    for (int row(start); row <= end; ++row)
    {
        const double value(m->data(m->index(row, DATA_COLUMN, rootIndex())).toDouble());
        const QString color(m->data(m->index(row, COLOR_COLUMN, rootIndex()), Qt::DecorationRole).toString());

        if (value > 0.0 && !color.isNull())
        {
            _validItems += 1;
            _totalValue += value;
        }    
    }

    QAbstractItemView::rowsInserted(parent, start, end);
}

void StackedBarView::rowsAboutToBeRemoved(const QModelIndex &parent, int start, int end)
{
    QAbstractItemModel * m(model());

    for (int row(start); row <= end; ++row)
    {
        const double value(m->data(m->index(row, DATA_COLUMN, rootIndex())).toDouble());
        const QString color(m->data(m->index(row, COLOR_COLUMN, rootIndex()), Qt::DecorationRole).toString());

        if (value > 0.0 && !color.isNull())
        {
            _validItems -= 1;
            _totalValue -= value;
        }    
    }

    QAbstractItemView::rowsAboutToBeRemoved(parent, start, end);
}

bool StackedBarView::edit(const QModelIndex&, EditTrigger, QEvent*)
{
    // disable editing for all
    return false;
}

QPixmap StackedBarView::legendIcon(const QModelIndex & index) const
{
    const QColor color = qvariant_cast<QColor>(index.data(Qt::DecorationRole));

    QLinearGradient gradient(0, 0, 0, 16);
    gradient.setColorAt(0.0, Qt::white);
    gradient.setColorAt(0.5, color);
    gradient.setColorAt(1.0, Qt::black);

    QPixmap pixmap(16, 16);
    QPainter painter(&pixmap);
    painter.fillRect(0, 0, 16, 16, gradient);

    return pixmap;
}
