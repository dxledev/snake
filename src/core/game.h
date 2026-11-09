#pragma once

#include <QPoint>
#include <QRandomGenerator>
#include <deque>
#include <functional>
#include <vector>

namespace snake {

enum class Direction { Up, Down, Left, Right };
enum class Phase { Ready, Running, Paused, Over };

// Pure game rules: no Qt widgets, deterministic with an injected random source.
class Game {
public:
    static constexpr int columns = 20;
    static constexpr int rows = 16;
    static constexpr int minimumInterval = 68;

    using Picker = std::function<int(int)>;  // returns an index in [0, n)

    explicit Game(Picker picker = {});

    void reset();
    // Queues a turn. The first press while Ready starts the game.
    void steer(Direction direction);
    void togglePause();
    void pause();
    void start();
    // Advances one step. Returns false when the game is not running.
    bool tick();

    Phase phase() const { return phase_; }
    int score() const { return score_; }
    int highScore() const { return highScore_; }
    void setHighScore(int value) { highScore_ = std::max(highScore_, value); }
    bool newHighScore() const { return newHighScore_; }
    bool won() const { return won_; }
    int interval() const;
    QPoint food() const { return food_; }
    bool hasFood() const { return hasFood_; }
    Direction heading() const { return heading_; }
    const std::deque<QPoint> &body() const { return body_; }

    // Test hook.
    void placeFood(QPoint cell) { food_ = cell; hasFood_ = true; }

private:
    Picker picker_;
    std::deque<QPoint> body_;
    Direction heading_ = Direction::Right;
    Direction queued_ = Direction::Right;
    Phase phase_ = Phase::Ready;
    QPoint food_;
    bool hasFood_ = false, newHighScore_ = false, won_ = false;
    int score_ = 0, highScore_ = 0;

    void spawnFood();
    void finish();
};

}
