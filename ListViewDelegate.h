#ifndef LISTVIEWDELEGATE_H
#define LISTVIEWDELEGATE_H

#include <QAbstractItemDelegate>
#include <QPainter>

class ListViewDelegate : public QAbstractItemDelegate
{
public:
    ListViewDelegate(QObject *parent = nullptr);
protected:
    void paint(QPainter *painter, QStyleOptionViewItem const &option, QModelIndex const &index) const;
    QSize sizeHint(QStyleOptionViewItem const &option, QModelIndex const &index) const;
private:
    double d_widthfraction;
    int d_radius;
    int d_toppadding;
    int d_bottompadding;
    int d_leftpadding;
    int d_rightpadding;
    int d_verticalmargin;
    int d_horizontalmargin;
    int d_pointerheight;
    int d_pointerwidth;
};

#endif // LISTVIEWDELEGATE_H
