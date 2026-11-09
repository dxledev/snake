#pragma once

#include "board.h"
#include "core/game.h"
#include "icon_button.h"
#include "modal.h"
#include "theme_watcher.h"

#include <QTimer>
#include <QWidget>

class QLabel;

namespace snake {

class Window : public QWidget {
    Q_OBJECT
public:
    Window(QString configDirectory, Theme theme, QWidget *parent = nullptr);
    Game &game() { return game_; }
    Board *board() const { return board_; }
    Modal *modal() const { return modal_; }
    void openHelp();
    void handleKey(int key);

protected:
    void keyPressEvent(QKeyEvent *) override;
    void resizeEvent(QResizeEvent *) override;
    void closeEvent(QCloseEvent *) override;
    void changeEvent(QEvent *) override;

private:
    QString configDirectory_;
    Game game_;
    QWidget *content_;
    Board *board_;
    Modal *modal_;
    ThemeWatcher *watcher_;
    QLabel *hint_;
    IconButton *play_, *restart_, *help_, *up_, *down_, *left_, *right_;
    QTimer timer_;
    int savedHighScore_ = 0;

    void applyTheme();
    void sync();
    void schedule();
    void onTick();
    void toggle();
    void restart();
    void saveScores();
    void loadScores();
};

}
