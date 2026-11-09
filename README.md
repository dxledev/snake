# Snake

A small, portable, standalone Snake game written in C++20 with Qt Widgets. Download it, build it,
theme it from the command line. No web runtime, no scripting engine, a single native binary.

Rules follow the classic phone game: a 20 × 16 grid, walls and your own body end the run, every
diamond is one point and speeds the snake up (165 ms per step, down to 68 ms at 25 points). Your
high score is saved. The window is freely resizable and the board scales in whole cells. The look
is flat and terminal-like: monospace type, a dotted cell grid, boxed frame, no gradients, and every
icon is drawn in your theme colors. Help opens as a modal inside the window over a soft blur.

## Controls

| Action | Keys |
| --- | --- |
| Steer | arrow keys, `W` `A` `S` `D`, or `H` `J` `K` `L` |
| Start / pause | `Space`, `Enter`, or `P` (also clicking the board) |
| Restart | `R` |
| How to play | `F1` |
| Close a panel / pause | `Escape` |

The game pauses when the window loses focus.

## Build and run

Requires CMake 3.21+, a C++20 compiler, and Qt 6.7+ (Core, Gui, Widgets, OpenGLWidgets, and Test
for the tests). On Linux, install Qt's Wayland platform plugin for native Wayland.

```bash
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
ctest --test-dir build --output-on-failure
./build/snake
```

Install under your own account, with a launcher entry and icon:

```bash
bash scripts/install-desktop.sh --dry-run
bash scripts/install-desktop.sh
```

The installer accepts `--prefix`, `--config-dir`, and `--apps-list` absolute paths. For a
conventional package use `cmake --install`. Configure with `-DSNAKE_SANITIZE=ON` for an
AddressSanitizer + UBSan build.

## Your colors

The palette lives at `$XDG_CONFIG_HOME/snake/theme.json` (usually `~/.config/snake/theme.json`).
Every CLI change rewrites that file atomically and any open window reloads it immediately. An
invalid update keeps the last good colors. `--config-dir /absolute/path` (or `SNAKE_CONFIG_DIR`)
selects a separate configuration.

```bash
snake theme show
snake theme set 'accent=#a9c7b0' 'snake=#8fd0a0'
snake theme set 'background=#e00e1e36'
snake theme set 'background=Qt.rgba(14 / 255, 30 / 255, 54 / 255, 0.88)'
snake theme import /absolute/path/palette.json
snake theme follow file /absolute/path/palette.json
snake theme follow symlink /absolute/path/palette-directory
snake theme sync
snake theme preset lofi --dry-run
```

`import` takes a snapshot; `follow` keeps tracking the source, including when a symlink is
re-pointed or the file is replaced, and `sync` re-reads it on demand. Manual `set` and `preset`
commands stop following. `--dry-run` prints the proposed theme without writing anything.
JSON, QML color-property files, and terminal (kitty) palettes are understood.

Tokens: `background`, `surface`, `surface_alt`, `text`, `muted`, `accent`, `accent_text`,
`border`, `grid`, `snake`, `snake_head`, `food`, `danger`.

**Alpha.** Eight-digit colors are Qt-style `#AARRGGBB` with alpha *first*, so `#e00e1e36` is the
color `#0e1e36` at 88% opacity. `Qt.rgba(r, g, b, a)` with 0–1 channels works too. Opacity is
applied to the window background when the display supports it; where it does not, only the alpha is
dropped and the RGB channels are left alone. Board cells are always opaque.

**Contrast.** When colors are derived from an imported or followed palette, text, snake, food,
and borders are nudged until they meet readable contrast against the background. Colors you set
by hand are used exactly as written.

## Files

- `~/.config/snake/theme.json` — palette
- `~/.config/snake/scores.json` — high score

## License

MIT. Gameplay rules are modeled on the classic phone game; no third-party code or art is included.
