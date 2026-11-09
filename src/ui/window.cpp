#include "window.h"

#include <QCloseEvent>
#include <QDir>
#include <QFile>
#include <QGuiApplication>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QKeyEvent>
#include <QLabel>
#include <QSaveFile>
#include <QVBoxLayout>

namespace snake {
namespace {

QString css(const QColor &color) {
    return QString("rgba(%1,%2,%3,%4)").arg(color.red()).arg(color.green()).arg(color.blue()).arg(color.alpha());
}

// Platforms whose windows can carry an alpha channel. Elsewhere only the alpha
// is dropped, never the RGB channels.
bool platformShowsAlpha() {
    if (qEnvironmentVariableIntValue("SNAKE_OPAQUE") == 1)
        return false;
    const auto name = QGuiApplication::platformName();
    return name.startsWith("wayland") || name == "xcb" || name == "offscreen";
}

const char *const kHint = "arrows / wasd / hjkl  move     space  pause     r  restart     f1  help";

Theme withoutAlpha(Theme theme) {
    for (auto it = theme.colors.begin(); it != theme.colors.end(); ++it)
        it.value() = QColor(it.value().red(), it.value().green(), it.value().blue());
    return theme;
}

}

Window::Window(QString directory, Theme theme, QWidget *parent)
    : QWidget(parent), configDirectory_(std::move(directory)) {
    setWindowTitle("Snake");
    setObjectName("snakeWindow");
    setAttribute(Qt::WA_TranslucentBackground);
    setMinimumSize(420, 420);
    resize(760, 700);
    setFocusPolicy(Qt::StrongFocus);

    auto *outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    content_ = new QWidget(this);
    content_->setObjectName("content");
    outer->addWidget(content_);
    auto *layout = new QVBoxLayout(content_);
    layout->setContentsMargins(20, 16, 20, 14);
    layout->setSpacing(10);

    board_ = new Board(game_, content_);
    layout->addWidget(board_, 1);

    auto *controls = new QHBoxLayout;
    controls->setSpacing(8);
    play_ = new IconButton(IconButton::Glyph::Play, "Start or pause (Space)", content_);
    restart_ = new IconButton(IconButton::Glyph::Restart, "Restart (R)", content_);
    help_ = new IconButton(IconButton::Glyph::Help, "How to play (F1)", content_);
    left_ = new IconButton(IconButton::Glyph::Left, "Steer left", content_);
    up_ = new IconButton(IconButton::Glyph::Up, "Steer up", content_);
    down_ = new IconButton(IconButton::Glyph::Down, "Steer down", content_);
    right_ = new IconButton(IconButton::Glyph::Right, "Steer right", content_);
    controls->addWidget(play_);
    controls->addWidget(restart_);
    controls->addWidget(help_);
    controls->addStretch();
    for (auto *b : {left_, up_, down_, right_})
        controls->addWidget(b);
    layout->addLayout(controls);

    hint_ = new QLabel(kHint, content_);
    hint_->setObjectName("hint");
    hint_->setAlignment(Qt::AlignCenter);
    hint_->setWordWrap(true);
    layout->addWidget(hint_);

    modal_ = new Modal(this, content_);
    watcher_ = new ThemeWatcher(configDirectory_ + "/theme.json", std::move(theme), this);

    connect(play_, &QAbstractButton::clicked, this, [this] { toggle(); });
    connect(restart_, &QAbstractButton::clicked, this, [this] { restart(); });
    connect(help_, &QAbstractButton::clicked, this, [this] { openHelp(); });
    connect(left_, &QAbstractButton::clicked, this, [this] { handleKey(Qt::Key_Left); });
    connect(up_, &QAbstractButton::clicked, this, [this] { handleKey(Qt::Key_Up); });
    connect(down_, &QAbstractButton::clicked, this, [this] { handleKey(Qt::Key_Down); });
    connect(right_, &QAbstractButton::clicked, this, [this] { handleKey(Qt::Key_Right); });
    connect(board_, &Board::activated, this, [this] {
        if (game_.phase() != Phase::Running)
            toggle();
    });
    connect(watcher_, &ThemeWatcher::changed, this, [this] {
        hint_->setText(kHint);
        applyTheme();
    });
    connect(watcher_, &ThemeWatcher::rejected, this, [this](const QString &error) {
        hint_->setText("Theme unchanged: " + error);
    });
    connect(modal_, &Modal::dismissed, this, [this] { setFocus(); sync(); });

    timer_.setTimerType(Qt::PreciseTimer);
    timer_.setSingleShot(true);
    connect(&timer_, &QTimer::timeout, this, &Window::onTick);

    loadScores();
    applyTheme();
    sync();
    setFocus();
}

void Window::applyTheme() {
    Theme theme = watcher_->theme();
    if (!platformShowsAlpha())
        theme = withoutAlpha(theme);
    auto c = [&](const char *key) { return css(theme.color(key)); };
    setStyleSheet(
        QString("QWidget#snakeWindow {background:transparent;}"
                "QWidget#content {background:%1;}"
                "QWidget {color:%2;font-family:'JetBrains Mono','DejaVu Sans Mono','Cascadia Mono','Liberation Mono',monospace;font-size:13px;}"
                "QLabel {background:transparent;}"
                "QLabel#hint {color:%3;font-size:12px;}"
                "QPushButton {background:%4;border:1px solid %5;padding:9px 14px;}"
                "QPushButton:hover {background:%6;border-color:%7;}"
                "QPushButton:focus {border:2px solid %7;padding:8px 13px;}"
                "QWidget#modalCard {background:%4;border:2px solid %7;}"
                "QWidget#modalCard QLabel {border:none;}"
                "QLabel#modalTitle {font-size:22px;font-weight:700;}"
                "QWidget#modalCard QPushButton {min-height:20px;}"
                "QWidget#modalCard QPushButton:focus {background:%7;color:%8;}"
                "QScrollBar:vertical {background:%4;width:10px;border:none;}"
                "QScrollBar::handle:vertical {background:%5;min-height:24px;}"
                "QScrollBar::add-line,QScrollBar::sub-line {height:0;}")
            .arg(c("background"), c("text"), c("muted"), c("surface"), c("border"), c("surface_alt"),
                 c("accent"), c("accent_text")));
    board_->setTheme(theme);
    modal_->setTheme(theme);
    for (auto *b : {play_, restart_, help_, up_, down_, left_, right_})
        b->setTheme(theme);
    setWindowIcon(QIcon::fromTheme("io.github.snake"));
}

void Window::sync() {
    play_->setGlyph(game_.phase() == Phase::Running ? IconButton::Glyph::Pause : IconButton::Glyph::Play);
    board_->update();
    if (game_.phase() == Phase::Running) {
        if (!timer_.isActive())
            schedule();
    } else {
        timer_.stop();
    }
}

void Window::schedule() {
    timer_.start(game_.interval());
}

void Window::onTick() {
    game_.tick();
    if (game_.phase() == Phase::Over)
        saveScores();
    sync();
    if (game_.phase() == Phase::Running)
        schedule();
}

void Window::toggle() {
    if (modal_->isVisible())
        return;
    game_.togglePause();
    sync();
}

void Window::restart() {
    if (modal_->isVisible())
        return;
    saveScores();
    game_.reset();
    sync();
}

void Window::openHelp() {
    if (modal_->isVisible())
        return;
    game_.pause();
    sync();
    modal_->open("How to play",
                 "Eat the diamonds to grow. Hitting a wall or your own body ends the run. "
                 "Each diamond is one point and the snake gets faster until it reaches top speed at 25 points.\n\n"
                 "Steer: arrow keys, WASD or HJKL.\n"
                 "Start or pause: Space, Enter or P.\n"
                 "Restart: R.\n"
                 "Help: F1. Close this panel with Escape.\n\n"
                 "The game pauses when this window loses focus.",
                 {"Back to the game"});
}

void Window::handleKey(int key) {
    if (modal_->isVisible())
        return;
    switch (key) {
    case Qt::Key_Up: case Qt::Key_W: case Qt::Key_K: game_.steer(Direction::Up); break;
    case Qt::Key_Down: case Qt::Key_S: case Qt::Key_J: game_.steer(Direction::Down); break;
    case Qt::Key_Left: case Qt::Key_A: case Qt::Key_H: game_.steer(Direction::Left); break;
    case Qt::Key_Right: case Qt::Key_D: case Qt::Key_L: game_.steer(Direction::Right); break;
    case Qt::Key_Space: case Qt::Key_Return: case Qt::Key_Enter: case Qt::Key_P: toggle(); return;
    case Qt::Key_R: restart(); return;
    case Qt::Key_F1: openHelp(); return;
    default: return;
    }
    sync();
}

void Window::keyPressEvent(QKeyEvent *event) {
    if (event->modifiers() & (Qt::ControlModifier | Qt::AltModifier | Qt::MetaModifier)) {
        QWidget::keyPressEvent(event);
        return;
    }
    if (event->key() == Qt::Key_Escape) {
        if (!modal_->isVisible() && game_.phase() == Phase::Running)
            toggle();
        return;
    }
    handleKey(event->key());
    event->accept();
}

void Window::changeEvent(QEvent *event) {
    QWidget::changeEvent(event);
    if (event->type() == QEvent::ActivationChange && !isActiveWindow() && game_.phase() == Phase::Running) {
        game_.pause();
        sync();
    }
}

void Window::resizeEvent(QResizeEvent *event) {
    QWidget::resizeEvent(event);
    if (modal_)
        modal_->setGeometry(rect());
    // The key legend only fits on one line when there is room; the buttons and F1 help cover small windows.
    if (hint_)
        hint_->setVisible(width() >= 620 || !hint_->text().startsWith("arrows"));
}

void Window::closeEvent(QCloseEvent *event) {
    saveScores();
    QWidget::closeEvent(event);
}

void Window::loadScores() {
    QFile file(configDirectory_ + "/scores.json");
    if (!file.open(QIODevice::ReadOnly) || file.size() > 4096)
        return;
    const auto document = QJsonDocument::fromJson(file.readAll());
    const int value = document.object().value("high_score").toInt(0);
    if (value > 0 && value < 100000) {
        savedHighScore_ = value;
        game_.setHighScore(value);
    }
}

void Window::saveScores() {
    if (game_.highScore() <= savedHighScore_)
        return;
    QDir().mkpath(configDirectory_);
    QSaveFile file(configDirectory_ + "/scores.json");
    if (!file.open(QIODevice::WriteOnly))
        return;
    file.write(QJsonDocument(QJsonObject{{"version", 1}, {"high_score", game_.highScore()}}).toJson());
    if (file.commit())
        savedHighScore_ = game_.highScore();
}

}
