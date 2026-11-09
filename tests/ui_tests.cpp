#include "core/theme.h"
#include "ui/window.h"

#include <QKeyEvent>
#include <QTemporaryDir>
#include <QtTest>

using namespace snake;

class UiTests : public QObject {
    Q_OBJECT
private slots:
    void keysSteerAndPause();
    void helpIsAnInWindowModalAndPauses();
    void cellsAreIntegerAndCentered();
    void highScorePersists();
    void themeChangeAndResizeWhileModalOpen();
    void destroyWithModalOpenMidGame();
};

static void press(QWidget *w, int key) { QTest::keyClick(w, Qt::Key(key)); }

void UiTests::keysSteerAndPause() {
    QTemporaryDir dir;
    Window window(dir.path(), presetTheme("lofi"));
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    press(&window, Qt::Key_K);
    QCOMPARE(window.game().phase(), Phase::Running);
    press(&window, Qt::Key_Space);
    QCOMPARE(window.game().phase(), Phase::Paused);
    press(&window, Qt::Key_R);
    QCOMPARE(window.game().phase(), Phase::Ready);
}

void UiTests::helpIsAnInWindowModalAndPauses() {
    QTemporaryDir dir;
    Window window(dir.path(), presetTheme("lofi"));
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    press(&window, Qt::Key_Up);
    window.openHelp();
    QVERIFY(window.modal()->isVisible());
    QCOMPARE(window.modal()->parentWidget(), &window);
    QVERIFY(!window.modal()->isWindow());
    QCOMPARE(window.game().phase(), Phase::Paused);
    press(window.modal(), Qt::Key_Escape);
    QTRY_VERIFY_WITH_TIMEOUT(!window.modal()->isVisible(), 2000);
}

void UiTests::cellsAreIntegerAndCentered() {
    QTemporaryDir dir;
    Window window(dir.path(), presetTheme("lofi"));
    window.resize(900, 760);
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    auto *board = window.board();
    const auto grid = board->gridRect();
    QCOMPARE(grid.width(), board->cellSize() * Game::columns);
    QCOMPARE(grid.height(), board->cellSize() * Game::rows);
    QVERIFY(board->rect().contains(grid));
    QVERIFY(std::abs(grid.center().x() - board->width() / 2) <= 1);
}

void UiTests::highScorePersists() {
    QTemporaryDir dir;
    {
        Window window(dir.path(), presetTheme("lofi"));
        window.game().placeFood(QPoint(7, 8));
        window.game().steer(Direction::Right);
        window.game().tick();
        window.close();
    }
    Window again(dir.path(), presetTheme("lofi"));
    QCOMPARE(again.game().highScore(), 1);
}

void UiTests::themeChangeAndResizeWhileModalOpen() {
    QTemporaryDir dir;
    Window window(dir.path(), presetTheme("lofi"));
    window.show();
    QVERIFY(QTest::qWaitForWindowExposed(&window));
    window.openHelp();
    QVERIFY(window.modal()->isVisible());
    saveTheme(dir.filePath("theme.json"), setColors(presetTheme("lofi"), {"accent=#ff8800", "background=#e00e1e36"}));
    QTest::qWait(700);  // watcher poll reloads while the modal is open
    window.resize(500, 520);
    QTest::qWait(50);
    window.resize(900, 700);
    QTest::qWait(50);
    QVERIFY(window.modal()->isVisible());
    QCOMPARE(window.modal()->size(), window.size());
    press(window.modal(), Qt::Key_Escape);
    QTRY_VERIFY_WITH_TIMEOUT(!window.modal()->isVisible(), 2000);
}

void UiTests::destroyWithModalOpenMidGame() {
    QTemporaryDir dir;
    auto *window = new Window(dir.path(), presetTheme("lofi"));
    window->show();
    QVERIFY(QTest::qWaitForWindowExposed(window));
    press(window, Qt::Key_Up);
    QTest::qWait(250);  // let a few ticks run
    window->openHelp();
    delete window;      // game, board and modal torn down together
    QTest::qWait(50);
}

QTEST_MAIN(UiTests)
#include "ui_tests.moc"
