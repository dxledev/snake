#include "core/storage.h"
#include "core/theme.h"
#include "core/theme_source.h"
#include "ui/theme_watcher.h"

#include <QDir>
#include <QFile>
#include <QSaveFile>
#include <cstdio>
#include <QJsonDocument>
#include <QSignalSpy>
#include <QTemporaryDir>
#include <QtTest>
#include <cmath>

using namespace snake;

namespace {

bool writeText(const QString &path, const QString &text) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
        return false;
    return file.write(text.toUtf8()) == text.toUtf8().size();
}

bool replaceSymlink(const QString &target, const QString &linkPath) {
    const QString temporaryPath = linkPath + ".new";
    QFile::remove(temporaryPath);
    if (!QFile::link(target, temporaryPath))
        return false;
    return std::rename(QFile::encodeName(temporaryPath).constData(), QFile::encodeName(linkPath).constData()) == 0;
}

QString palette(const QString &accent, const QString &background = "#15191d") {
    return QString(R"(pragma Singleton
import QtQuick
Singleton {
    readonly property color background: "%1"
    readonly property color foreground: "#e8e1d8"
    readonly property color primary: "%2"
    readonly property color red: "#e27676"
})").arg(background, accent);
}

bool atomicWrite(const QString &path, const QString &text) {
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    file.write(text.toUtf8());
    return file.commit();
}

double lum(const QColor &c) {
    auto lin = [](double v) { return v <= .04045 ? v / 12.92 : std::pow((v + .055) / 1.055, 2.4); };
    return .2126 * lin(c.redF()) + .7152 * lin(c.greenF()) + .0722 * lin(c.blueF());
}

double ratio(const QColor &a, const QColor &b) {
    const double x = lum(a), y = lum(b);
    return (std::max(x, y) + .05) / (std::min(x, y) + .05);
}

}

class ThemeTests : public QObject {
    Q_OBJECT
private slots:
    void presetRoundTrips();
    void alphaFirstAndQtRgbaKeepRgbChannels();
    void importedPaletteIsContrastSafe();
    void lightPaletteIsContrastSafe();
    void foodSeparatesFromSnake();
    void unknownKeysAndSourcesAreRejected();
    void watcherReloadsAndKeepsLastValidTheme();
    void watcherFollowsRepointedSymlinkAndRegeneratedPalette();
    void watcherFollowsSymlinkChain();
};



void ThemeTests::presetRoundTrips() {
    const auto theme = presetTheme("lofi");
    QCOMPARE(parseTheme(theme.toJson()), theme);
    QVERIFY(ratio(theme.color("text"), theme.color("background")) >= 7);
    QVERIFY(ratio(theme.color("snake"), theme.color("background")) >= 4.5);
    QVERIFY(ratio(theme.color("food"), theme.color("background")) >= 4.5);
}

void ThemeTests::alphaFirstAndQtRgbaKeepRgbChannels() {
    auto theme = setColors(presetTheme("lofi"), {"background=#e00e1e36"});
    QCOMPARE(theme.color("background").alpha(), 0xe0);
    QCOMPARE(theme.color("background").name(), QString("#0e1e36"));
    theme = setColors(presetTheme("lofi"), {"background=Qt.rgba(14 / 255, 30 / 255, 54 / 255, 0.88)"});
    QCOMPARE(theme.color("background").name(), QString("#0e1e36"));
    QCOMPARE(theme.color("background").alpha(), 0xe0);
    QCOMPARE(theme.toJson().value("colors").toObject().value("background").toString(), QString("#e00e1e36"));
    QVERIFY_EXCEPTION_THROWN(setColors(presetTheme("lofi"), {"background=#bad"}), std::exception);
}

void ThemeTests::importedPaletteIsContrastSafe() {
    QTemporaryDir dir;
    const auto file = dir.filePath("Colors.qml");
    QVERIFY(writeText(file, R"(pragma Singleton
import QtQuick
Singleton {
    readonly property color background: "#e00e1e36"
    readonly property color foreground: "#3a4a5a"
    readonly property color primary: "#2a3a4a"
    readonly property color red: "#401010"
})"));
    const auto theme = importPalette(file);
    const QColor board(theme.color("background").red(), theme.color("background").green(), theme.color("background").blue());
    QCOMPARE(board.name(), QString("#0e1e36"));
    QCOMPARE(theme.color("background").alpha(), 0xe0);
    QVERIFY(ratio(theme.color("text"), board) >= 7);
    QVERIFY(ratio(theme.color("accent"), board) >= 4.5);
    QVERIFY(ratio(theme.color("snake"), board) >= 4.5);
    QVERIFY(ratio(theme.color("snake_head"), board) >= 7);
    QVERIFY(ratio(theme.color("food"), board) >= 4.5);
    QVERIFY(ratio(theme.color("danger"), board) >= 4.5);
    QVERIFY(ratio(theme.color("border"), board) >= 3);
}

void ThemeTests::lightPaletteIsContrastSafe() {
    QTemporaryDir dir;
    const auto file = dir.filePath("palette.json");
    QVERIFY(writeText(file, R"({"background":"#f2efe6","foreground":"#cfcabd","primary":"#d8d0b8","red":"#f0c0c0"})"));
    const auto theme = importPalette(file);
    const QColor board = theme.color("background");
    QVERIFY(ratio(theme.color("text"), board) >= 7);
    QVERIFY(ratio(theme.color("snake"), board) >= 4.5);
    QVERIFY(ratio(theme.color("food"), board) >= 4.5);
    QVERIFY(ratio(theme.color("muted"), theme.color("surface")) >= 4.5);
}

void ThemeTests::foodSeparatesFromSnake() {
    QTemporaryDir dir;
    const auto file = dir.filePath("palette.json");
    // Accent and red are near-identical oranges; food must still read apart from the snake.
    QVERIFY(writeText(file, R"({"background":"#130d09","foreground":"#f7e1d4","primary":"#f3bb91","red":"#ff9b64"})"));
    const auto theme = importPalette(file);
    const double hue = std::abs(theme.color("snake").hslHueF() - theme.color("food").hslHueF()) * 360.0;
    QVERIFY(std::min(hue, 360.0 - hue) >= 60.0);
    QVERIFY(ratio(theme.color("food"), theme.color("background")) >= 4.5);
    QVERIFY(ratio(theme.color("muted"), theme.color("background")) >= 4.5);
}

void ThemeTests::unknownKeysAndSourcesAreRejected() {
    QVERIFY_EXCEPTION_THROWN(setColors(presetTheme("lofi"), {"card=#112233"}), std::exception);
    QVERIFY_EXCEPTION_THROWN(followTheme("desktop", "/tmp"), std::exception);
    QVERIFY_EXCEPTION_THROWN(presetTheme("nope"), std::exception);
}

void ThemeTests::watcherReloadsAndKeepsLastValidTheme() {
    QTemporaryDir dir;
    const auto path = dir.filePath("theme.json");
    auto theme = ensureTheme(path);
    ThemeWatcher watcher(path, theme);
    QSignalSpy changed(&watcher, &ThemeWatcher::changed);
    QSignalSpy rejected(&watcher, &ThemeWatcher::rejected);
    saveTheme(path, setColors(theme, {"accent=#ff8800"}));
    QTRY_VERIFY_WITH_TIMEOUT(changed.count() >= 1, 3000);
    const auto accepted = watcher.theme().color("accent");
    QVERIFY(writeText(path, "{ not json"));
    QTRY_VERIFY_WITH_TIMEOUT(rejected.count() >= 1, 3000);
    QCOMPARE(watcher.theme().color("accent"), accepted);
}

void ThemeTests::watcherFollowsRepointedSymlinkAndRegeneratedPalette() {
    QTemporaryDir dir;
    const auto a = dir.filePath("a"), b = dir.filePath("b"), selector = dir.filePath("selector");
    QVERIFY(QDir().mkpath(a) && QDir().mkpath(b));
    QVERIFY(writeText(a + "/Colors.qml", palette("#80a0ff")));
    QVERIFY(writeText(b + "/Colors.qml", palette("#ffa040")));
    QVERIFY(QFile::link(a, selector));
    const auto config = dir.filePath("config/theme.json");
    saveTheme(config, followTheme("symlink", selector));
    ThemeWatcher watcher(config, loadTheme(config));
    QSignalSpy changed(&watcher, &ThemeWatcher::changed);
    // Atomic re-point: a shell-mode or static/dynamic switch.
    QVERIFY(replaceSymlink(b, selector));
    QTRY_VERIFY_WITH_TIMEOUT(changed.count() >= 1, 4000);
    QCOMPARE(watcher.theme().color("accent").name(), QString("#ffa040"));
    // In-place regeneration: a new wallpaper rewriting the active palette.
    const int before = changed.count();
    QVERIFY(atomicWrite(b + "/Colors.qml", palette("#40ffa0")));
    QTRY_VERIFY_WITH_TIMEOUT(changed.count() > before, 4000);
    QCOMPARE(watcher.theme().color("accent").name(), QString("#40ffa0"));
}

void ThemeTests::watcherFollowsSymlinkChain() {
    QTemporaryDir dir;
    const auto a = dir.filePath("a"), b = dir.filePath("b");
    QVERIFY(QDir().mkpath(a) && QDir().mkpath(b));
    QVERIFY(writeText(a + "/Colors.qml", palette("#80a0ff")));
    QVERIFY(writeText(b + "/Colors.qml", palette("#ffa040")));
    const auto current = dir.filePath("current"), selector = dir.filePath("selector");
    QVERIFY(QFile::link(a, current));
    QVERIFY(QFile::link(current, selector));  // selector -> current -> a
    const auto config = dir.filePath("config/theme.json");
    saveTheme(config, followTheme("symlink", selector));
    ThemeWatcher watcher(config, loadTheme(config));
    QSignalSpy changed(&watcher, &ThemeWatcher::changed);
    QVERIFY(replaceSymlink(b, current));  // only the second hop moves
    QTRY_VERIFY_WITH_TIMEOUT(changed.count() >= 1, 4000);
    QCOMPARE(watcher.theme().color("accent").name(), QString("#ffa040"));
}

QTEST_MAIN(ThemeTests)
#include "theme_tests.moc"
