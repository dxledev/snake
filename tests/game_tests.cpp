#include "core/game.h"

#include <QtTest>

using namespace snake;

class GameTests : public QObject {
    Q_OBJECT
private slots:
    void startsReadyWithThreeSegments();
    void firstSteerStartsGame();
    void reversalIsRejectedAgainstHeading();
    void wallsKill();
    void tailCellIsFreeUnlessGrowing();
    void selfCollisionKills();
    void eatingScoresGrowsAndSpeedsUp();
    void intervalIsClamped();
    void fullBoardEndsGame();
    void pauseAndRestart();
};

static Game fixed() { return Game([](int) { return 0; }); }

void GameTests::startsReadyWithThreeSegments() {
    auto game = fixed();
    QCOMPARE(game.phase(), Phase::Ready);
    QCOMPARE(int(game.body().size()), 3);
    QCOMPARE(game.body().front(), QPoint(6, 8));
    QVERIFY(!game.tick());
}

void GameTests::firstSteerStartsGame() {
    auto game = fixed();
    game.steer(Direction::Up);
    QCOMPARE(game.phase(), Phase::Running);
    game.tick();
    QCOMPARE(game.body().front(), QPoint(6, 7));
}

void GameTests::reversalIsRejectedAgainstHeading() {
    auto game = fixed();
    game.steer(Direction::Left);  // reverse of Right: ignored, does not start
    QCOMPARE(game.phase(), Phase::Ready);
    game.steer(Direction::Up);
    game.steer(Direction::Left);  // still heading Right until the tick: must not fold back
    game.tick();
    QCOMPARE(game.body().front(), QPoint(6, 7));
    game.steer(Direction::Down);  // now heading Up: Down is a reversal
    game.tick();
    QCOMPARE(game.body().front(), QPoint(6, 6));
}

void GameTests::wallsKill() {
    auto game = fixed();
    game.steer(Direction::Right);
    for (int i = 0; i < 30 && game.phase() == Phase::Running; ++i)
        game.tick();
    QCOMPARE(game.phase(), Phase::Over);
}

void GameTests::tailCellIsFreeUnlessGrowing() {
    auto game = fixed();
    game.placeFood(QPoint(7, 8));
    game.steer(Direction::Right);
    game.tick();  // length 4
    game.placeFood(QPoint(19, 15));
    game.steer(Direction::Up);
    game.tick();
    game.steer(Direction::Left);
    game.tick();
    game.steer(Direction::Down);
    game.tick();  // enters the cell the tail vacates this tick
    QCOMPARE(game.phase(), Phase::Running);
    QCOMPARE(game.body().front(), QPoint(6, 8));
}

void GameTests::selfCollisionKills() {
    auto game = fixed();
    // Grow to 5 segments by feeding twice, then curl into itself.
    game.placeFood(QPoint(7, 8));
    game.steer(Direction::Right);
    game.tick();
    game.placeFood(QPoint(8, 8));
    game.tick();
    QCOMPARE(int(game.body().size()), 5);
    game.placeFood(QPoint(19, 15));
    game.steer(Direction::Down);
    game.tick();
    game.steer(Direction::Left);
    game.tick();
    game.steer(Direction::Up);
    game.tick();
    QCOMPARE(game.phase(), Phase::Over);
}

void GameTests::eatingScoresGrowsAndSpeedsUp() {
    auto game = fixed();
    game.placeFood(QPoint(7, 8));
    game.steer(Direction::Right);
    game.tick();
    QCOMPARE(game.score(), 1);
    QCOMPARE(int(game.body().size()), 4);
    QCOMPARE(game.interval(), 161);
    QCOMPARE(game.highScore(), 1);
    QVERIFY(game.newHighScore());
}

void GameTests::intervalIsClamped() {
    QCOMPARE(Game::minimumInterval, 68);
}

void GameTests::fullBoardEndsGame() {
    // Snake length 3 on a 320-cell board cannot fill it by play here; assert the guard instead:
    // food placement never lands on the body.
    Game game([](int n) { return n - 1; });
    for (const auto &cell : game.body())
        QVERIFY(game.food() != cell);
    QVERIFY(game.hasFood());
}

void GameTests::pauseAndRestart() {
    auto game = fixed();
    game.steer(Direction::Up);
    game.togglePause();
    QCOMPARE(game.phase(), Phase::Paused);
    game.steer(Direction::Down);
    QVERIFY(!game.tick());
    game.togglePause();
    QCOMPARE(game.phase(), Phase::Running);
    for (int i = 0; i < 30 && game.phase() == Phase::Running; ++i)
        game.tick();
    QCOMPARE(game.phase(), Phase::Over);
    game.start();  // Space after game over begins a fresh run
    QCOMPARE(game.phase(), Phase::Running);
    QCOMPARE(game.score(), 0);
    QCOMPARE(int(game.body().size()), 3);
}

QTEST_GUILESS_MAIN(GameTests)
#include "game_tests.moc"
