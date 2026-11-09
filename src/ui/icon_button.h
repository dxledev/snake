#pragma once

#include "core/theme.h"

#include <QAbstractButton>

namespace snake {

// A flat, square, TUI-style button whose glyph is a vector path painted in
// theme colors, so it always follows the palette.
class IconButton : public QAbstractButton {
    Q_OBJECT
public:
    enum class Glyph { Up, Down, Left, Right, Play, Pause, Restart, Help };
    IconButton(Glyph glyph, const QString &label, QWidget *parent = nullptr);
    void setGlyph(Glyph glyph);
    Glyph glyph() const { return glyph_; }
    void setTheme(const Theme &theme);
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent *) override;
    void enterEvent(QEnterEvent *) override;
    void leaveEvent(QEvent *) override;

private:
    Glyph glyph_;
    Theme theme_;
    bool hover_ = false;
};

}
