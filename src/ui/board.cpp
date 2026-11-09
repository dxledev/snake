#include "board.h"

#include <QFontMetrics>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <algorithm>

namespace snake {
namespace {
constexpr int frameMargin = 14;
constexpr int topBand = 26;
constexpr int bottomBand = 26;
}

Board::Board(Game &game, QWidget *parent) : QWidget(parent), game_(game) {
    setObjectName("board");
    setFocusPolicy(Qt::NoFocus);
    setMinimumSize(20 * 8 + 2 * frameMargin + 4, 16 * 8 + topBand + bottomBand + frameMargin);
}

void Board::setTheme(const Theme &theme) {
    theme_ = theme;
    update();
}

QSize Board::minimumSizeHint() const {
    return minimumSize();
}

QFont Board::font(double pixels, bool bold) const {
    QFont f;
    f.setFamilies({"JetBrains Mono", "DejaVu Sans Mono", "Cascadia Mono", "Liberation Mono", "monospace"});
    f.setStyleHint(QFont::Monospace);
    f.setPixelSize(std::max(10, int(pixels)));
    f.setBold(bold);
    return f;
}

int Board::cellSize() const {
    const int w = (width() - 2 * frameMargin - 4) / Game::columns;
    const int h = (height() - topBand - bottomBand - frameMargin - 4) / Game::rows;
    return std::max(4, std::min(w, h));
}

QRect Board::gridRect() const {
    const int cell = cellSize();
    const QSize size(cell * Game::columns, cell * Game::rows);
    const int x = (width() - size.width()) / 2;
    const int y = topBand + (height() - topBand - bottomBand - size.height()) / 2;
    return QRect(QPoint(x, y), size);
}

void Board::drawLabel(QPainter &p, const QRect &where, const QString &text, const QColor &ink, Qt::Alignment align) const {
    p.setPen(ink);
    p.drawText(where, align | Qt::AlignVCenter | Qt::TextSingleLine, text);
}

void Board::mousePressEvent(QMouseEvent *event) {
    event->accept();
    emit activated();
}

void Board::paintEvent(QPaintEvent *) {
    if (theme_.colors.isEmpty())
        return;
    QPainter p(this);
    const QRect grid = gridRect();
    const int cell = cellSize();
    const QColor text = theme_.color("text"), muted = theme_.color("muted"), border = theme_.color("border");
    const QColor background = theme_.color("background");
    const QColor opaqueBackground(background.red(), background.green(), background.blue());

    // Frame: a single crisp line with room for the labels in its top and bottom edges.
    const QRect frame = grid.adjusted(-8, -8, 8, 8);
    p.setPen(QPen(border, 1));
    p.setBrush(Qt::NoBrush);
    p.drawRect(frame.adjusted(0, 0, -1, -1));

    // Board floor: slightly lifted so cells read against the window.
    p.fillRect(grid, theme_.color("surface"));

    // Grid: a dot at every cell center, a TUI "empty cell".
    p.setPen(Qt::NoPen);
    p.setBrush(theme_.color("grid"));
    const int dot = std::max(2, cell / 8);
    for (int y = 0; y < Game::rows; ++y)
        for (int x = 0; x < Game::columns; ++x)
            p.drawRect(grid.x() + x * cell + (cell - dot) / 2, grid.y() + y * cell + (cell - dot) / 2, dot, dot);

    // Food: a diamond, sharp-edged and filled.
    if (game_.hasFood()) {
        const QPointF c(grid.x() + (game_.food().x() + .5) * cell, grid.y() + (game_.food().y() + .5) * cell);
        const double r = cell * .38;
        QPainterPath diamond;
        diamond.moveTo(c.x(), c.y() - r);
        diamond.lineTo(c.x() + r, c.y());
        diamond.lineTo(c.x(), c.y() + r);
        diamond.lineTo(c.x() - r, c.y());
        diamond.closeSubpath();
        p.setRenderHint(QPainter::Antialiasing);
        p.setBrush(theme_.color("food"));
        p.drawPath(diamond);
        p.setRenderHint(QPainter::Antialiasing, false);
    }

    // Snake: solid blocks with a 1px gutter, so every cell stays countable.
    const int gutter = std::max(1, cell / 12);
    const bool over = game_.phase() == Phase::Over;
    bool first = true;
    for (const QPoint &segment : game_.body()) {
        const QRect block(grid.x() + segment.x() * cell + gutter, grid.y() + segment.y() * cell + gutter,
                          cell - 2 * gutter, cell - 2 * gutter);
        p.fillRect(block, first ? theme_.color(over ? "danger" : "snake_head") : theme_.color("snake"));
        if (first) {
            // Two "eyes" cut out of the head so heading stays legible.
            const int eye = std::max(2, cell / 6);
            QPoint a, b;
            const int lo = cell / 4, hi = cell - cell / 4 - eye;
            switch (game_.heading()) {
            case Direction::Right: a = {hi, lo}; b = {hi, hi}; break;
            case Direction::Left: a = {lo, lo}; b = {lo, hi}; break;
            case Direction::Up: a = {lo, lo}; b = {hi, lo}; break;
            case Direction::Down: a = {lo, hi}; b = {hi, hi}; break;
            }
            p.fillRect(QRect(grid.topLeft() + QPoint(segment.x() * cell, segment.y() * cell) + a, QSize(eye, eye)), opaqueBackground);
            p.fillRect(QRect(grid.topLeft() + QPoint(segment.x() * cell, segment.y() * cell) + b, QSize(eye, eye)), opaqueBackground);
        }
        first = false;
    }

    // Edge labels cut into the frame (like a TUI box title).
    const double pixels = std::clamp(cell * .55, 11.0, 17.0);
    p.setFont(font(pixels, true));
    const QFontMetrics fm(p.font());
    auto cutLabel = [&](const QString &label, int y, bool left, const QColor &ink) {
        const int w = fm.horizontalAdvance(label) + 14;
        const QRect where(left ? frame.left() + 12 : frame.right() - 12 - w, y - fm.height() / 2, w, fm.height());
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.fillRect(where.adjusted(0, 1, 0, -1), background);
        p.setCompositionMode(QPainter::CompositionMode_SourceOver);
        drawLabel(p, where, label, ink, Qt::AlignCenter);
    };
    cutLabel("SNAKE", frame.top(), true, text);
    cutLabel(QString("SCORE %1").arg(game_.score(), 4, 10, QChar('0')), frame.top(), false, text);
    p.setFont(font(pixels));
    cutLabel(QString("HI %1").arg(game_.highScore(), 4, 10, QChar('0')), frame.bottom(), false, muted);
    const char *status = "READY";
    switch (game_.phase()) {
    case Phase::Ready: status = "READY"; break;
    case Phase::Running: status = "PLAYING"; break;
    case Phase::Paused: status = "PAUSED"; break;
    case Phase::Over: status = "GAME OVER"; break;
    }
    cutLabel(QString::fromLatin1(status), frame.bottom(), true, game_.phase() == Phase::Over ? theme_.color("danger") : muted);

    if (game_.phase() != Phase::Running)
        drawOverlay(p, grid);
}

void Board::drawOverlay(QPainter &p, const QRect &grid) const {
    QString title, line;
    switch (game_.phase()) {
    case Phase::Ready: title = "SNAKE"; line = "PRESS AN ARROW KEY TO PLAY"; break;
    case Phase::Paused: title = "PAUSED"; line = "SPACE TO RESUME"; break;
    case Phase::Over:
        title = game_.won() ? "BOARD CLEARED" : "GAME OVER";
        line = game_.newHighScore() ? QString("NEW HIGH SCORE %1  -  SPACE TO PLAY").arg(game_.score())
                                    : QString("SCORE %1  -  SPACE TO PLAY AGAIN").arg(game_.score());
        break;
    case Phase::Running: return;
    }
    const double pixels = std::clamp(cellSize() * .6, 12.0, 20.0);
    QFont big = font(pixels * 1.5, true), small = font(pixels);
    const QFontMetrics bigMetrics(big), smallMetrics(small);
    const int width = std::min(grid.width() - 16, std::max(bigMetrics.horizontalAdvance(title), smallMetrics.horizontalAdvance(line)) + 48);
    const int height = bigMetrics.height() + smallMetrics.height() + 40;
    // Keep the banner clear of the snake: sit in the half the head is not in.
    const bool headHigh = !game_.body().empty() && game_.body().front().y() < Game::rows / 2;
    const int centerY = headHigh ? grid.top() + grid.height() * 3 / 4 : grid.top() + grid.height() / 4;
    const QRect box(grid.center().x() - width / 2, centerY - height / 2, width, height);
    p.setRenderHint(QPainter::Antialiasing, false);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(theme_.color("background").red(), theme_.color("background").green(), theme_.color("background").blue()));
    p.drawRect(box);
    p.setBrush(Qt::NoBrush);
    p.setPen(QPen(theme_.color(game_.phase() == Phase::Over ? "danger" : "accent"), 2));
    p.drawRect(box.adjusted(1, 1, -1, -1));
    p.setFont(big);
    p.setPen(theme_.color("text"));
    p.drawText(QRect(box.left(), box.top() + 14, box.width(), bigMetrics.height()), Qt::AlignCenter, title);
    p.setFont(small);
    p.setPen(theme_.color("muted"));
    p.drawText(QRect(box.left() + 8, box.top() + 20 + bigMetrics.height(), box.width() - 16, smallMetrics.height() + 6), Qt::AlignCenter, smallMetrics.elidedText(line, Qt::ElideRight, box.width() - 16));
}

}
