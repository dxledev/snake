#pragma once

#include "core/game.h"
#include "core/theme.h"

#include <QWidget>

namespace snake {

// Paints the playfield as a terminal-style cell grid with a box frame.
class Board : public QWidget {
    Q_OBJECT
public:
    explicit Board(Game &game, QWidget *parent = nullptr);
    void setTheme(const Theme &theme);
    QSize minimumSizeHint() const override;
    int cellSize() const;
    QRect gridRect() const;

signals:
    void activated();  // the board was clicked

protected:
    void paintEvent(QPaintEvent *) override;
    void mousePressEvent(QMouseEvent *) override;

private:
    Game &game_;
    Theme theme_;
    QFont font(double pixels, bool bold = false) const;
    void drawLabel(QPainter &p, const QRect &where, const QString &text, const QColor &ink, Qt::Alignment align) const;
    void drawOverlay(QPainter &p, const QRect &grid) const;
};

}
