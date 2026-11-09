#include "game.h"

#include <algorithm>

namespace snake {
namespace {

QPoint step(QPoint cell, Direction direction) {
    switch (direction) {
    case Direction::Up: return {cell.x(), cell.y() - 1};
    case Direction::Down: return {cell.x(), cell.y() + 1};
    case Direction::Left: return {cell.x() - 1, cell.y()};
    case Direction::Right: return {cell.x() + 1, cell.y()};
    }
    return cell;
}

bool opposite(Direction a, Direction b) {
    return (a == Direction::Up && b == Direction::Down) || (a == Direction::Down && b == Direction::Up)
        || (a == Direction::Left && b == Direction::Right) || (a == Direction::Right && b == Direction::Left);
}

}

Game::Game(Picker picker) : picker_(std::move(picker)) {
    if (!picker_)
        picker_ = [](int n) { return int(QRandomGenerator::global()->bounded(n)); };
    reset();
}

void Game::reset() {
    body_ = {QPoint(6, 8), QPoint(5, 8), QPoint(4, 8)};
    heading_ = queued_ = Direction::Right;
    phase_ = Phase::Ready;
    score_ = 0;
    newHighScore_ = won_ = false;
    hasFood_ = false;
    spawnFood();
}

int Game::interval() const {
    return std::max(minimumInterval, 165 - score_ * 4);
}

void Game::steer(Direction direction) {
    if (phase_ == Phase::Over || phase_ == Phase::Paused)
        return;
    // Reversal is judged against the heading actually travelled, so two quick
    // presses between ticks cannot fold the snake back onto itself.
    if (opposite(direction, heading_))
        return;
    queued_ = direction;
    if (phase_ == Phase::Ready)
        phase_ = Phase::Running;
}

void Game::start() {
    if (phase_ == Phase::Over)
        reset();
    if (phase_ == Phase::Ready || phase_ == Phase::Paused)
        phase_ = Phase::Running;
}

void Game::pause() {
    if (phase_ == Phase::Running)
        phase_ = Phase::Paused;
}

void Game::togglePause() {
    if (phase_ == Phase::Running)
        phase_ = Phase::Paused;
    else
        start();
}

void Game::spawnFood() {
    std::vector<QPoint> free;
    free.reserve(columns * rows);
    for (int y = 0; y < rows; ++y)
        for (int x = 0; x < columns; ++x) {
            const QPoint cell(x, y);
            if (std::find(body_.begin(), body_.end(), cell) == body_.end())
                free.push_back(cell);
        }
    if (free.empty()) {
        hasFood_ = false;
        won_ = true;
        finish();
        return;
    }
    const int index = std::clamp(picker_(int(free.size())), 0, int(free.size()) - 1);
    food_ = free[size_t(index)];
    hasFood_ = true;
}

void Game::finish() {
    phase_ = Phase::Over;
    if (score_ > highScore_) {
        highScore_ = score_;
        newHighScore_ = true;
    }
}

bool Game::tick() {
    if (phase_ != Phase::Running)
        return false;
    heading_ = queued_;
    const QPoint next = step(body_.front(), heading_);
    if (next.x() < 0 || next.y() < 0 || next.x() >= columns || next.y() >= rows) {
        finish();
        return true;
    }
    const bool eats = hasFood_ && next == food_;
    // The tail leaves its cell this tick unless the snake is growing.
    const auto end = eats ? body_.end() : body_.end() - 1;
    if (std::find(body_.begin(), end, next) != end) {
        finish();
        return true;
    }
    body_.push_front(next);
    if (eats) {
        ++score_;
        if (score_ > highScore_) {
            highScore_ = score_;
            newHighScore_ = true;
        }
        spawnFood();
    } else {
        body_.pop_back();
    }
    return true;
}

}
