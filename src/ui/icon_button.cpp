#include "icon_button.h"

#include <QPainter>
#include <QPainterPath>

namespace snake {

IconButton::IconButton(Glyph glyph, const QString &label, QWidget *parent)
    : QAbstractButton(parent), glyph_(glyph) {
    setAccessibleName(label);
    setToolTip(label);
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::TabFocus);
    setFixedHeight(36);
}

void IconButton::setGlyph(Glyph glyph) {
    glyph_ = glyph;
    update();
}

void IconButton::setTheme(const Theme &theme) {
    theme_ = theme;
    update();
}

QSize IconButton::sizeHint() const {
    return {44, 36};
}

void IconButton::enterEvent(QEnterEvent *) {
    hover_ = true;
    update();
}

void IconButton::leaveEvent(QEvent *) {
    hover_ = false;
    update();
}

void IconButton::paintEvent(QPaintEvent *) {
    if (theme_.colors.isEmpty())
        return;
    QPainter p(this);
    const QRectF box = QRectF(rect()).adjusted(.5, .5, -.5, -.5);
    const bool down = isDown();
    const QColor fill = !isEnabled() ? theme_.color("surface")
        : down ? theme_.color("accent") : hover_ ? theme_.color("surface_alt") : theme_.color("surface");
    const QColor ink = !isEnabled() ? theme_.color("muted") : down ? theme_.color("accent_text") : theme_.color("text");
    p.fillRect(box, fill);
    p.setPen(QPen(hasFocus() || hover_ ? theme_.color("accent") : theme_.color("border"), hasFocus() ? 2 : 1));
    p.setBrush(Qt::NoBrush);
    p.drawRect(hasFocus() ? box.adjusted(1, 1, -1, -1) : box);

    const QPointF c = rect().center();
    const double u = qMin(width(), height()) / 36.0 * 6.0;  // glyph half-size
    QPainterPath path;
    switch (glyph_) {
    case Glyph::Up: path.moveTo(c.x(), c.y() - u); path.lineTo(c.x() + u, c.y() + u * .8); path.lineTo(c.x() - u, c.y() + u * .8); break;
    case Glyph::Down: path.moveTo(c.x(), c.y() + u); path.lineTo(c.x() + u, c.y() - u * .8); path.lineTo(c.x() - u, c.y() - u * .8); break;
    case Glyph::Left: path.moveTo(c.x() - u, c.y()); path.lineTo(c.x() + u * .8, c.y() - u); path.lineTo(c.x() + u * .8, c.y() + u); break;
    case Glyph::Right: path.moveTo(c.x() + u, c.y()); path.lineTo(c.x() - u * .8, c.y() - u); path.lineTo(c.x() - u * .8, c.y() + u); break;
    case Glyph::Play: path.moveTo(c.x() - u * .7, c.y() - u); path.lineTo(c.x() + u, c.y()); path.lineTo(c.x() - u * .7, c.y() + u); break;
    case Glyph::Pause:
        path.addRect(QRectF(c.x() - u * .85, c.y() - u, u * .6, u * 2));
        path.addRect(QRectF(c.x() + u * .25, c.y() - u, u * .6, u * 2));
        break;
    case Glyph::Restart: {
        QPainterPath arc;
        const QRectF circle(c.x() - u, c.y() - u, u * 2, u * 2);
        arc.arcMoveTo(circle, 60);
        arc.arcTo(circle, 60, 290);
        QPen pen(ink, 2.2, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        p.drawPath(arc);
        const QPointF tip = arc.pointAtPercent(0);
        QPainterPath head;
        head.moveTo(tip.x() - u * .15, tip.y() - u * .75);
        head.lineTo(tip.x() + u * .75, tip.y() - u * .1);
        head.lineTo(tip.x() - u * .5, tip.y() + u * .35);
        head.closeSubpath();
        p.setPen(Qt::NoPen);
        p.setBrush(ink);
        p.drawPath(head);
        return;
    }
    case Glyph::Help: {
        QPen pen(ink, 2.2, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin);
        p.setPen(pen);
        p.setBrush(Qt::NoBrush);
        p.setRenderHint(QPainter::Antialiasing);
        QPainterPath mark;
        mark.moveTo(c.x() - u * .6, c.y() - u * .4);
        mark.arcTo(QRectF(c.x() - u * .6, c.y() - u, u * 1.2, u * 1.1), 180, -200);
        mark.lineTo(c.x(), c.y() + u * .35);
        p.drawPath(mark);
        p.setPen(Qt::NoPen);
        p.setBrush(ink);
        p.drawRect(QRectF(c.x() - u * .17, c.y() + u * .65, u * .34, u * .34));
        return;
    }
    }
    path.closeSubpath();
    p.setRenderHint(QPainter::Antialiasing);
    p.setPen(Qt::NoPen);
    p.setBrush(ink);
    p.drawPath(path);
}

}
