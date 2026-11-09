# Product

<!-- impeccable:product-schema 1 -->

## Platform

web

<!-- Note: actually a native Qt6 Widgets C++20 desktop app for Wayland; "web" is the nearest schema value. Inferred from the brief and confirmed by the user. -->

## Stack

C++20, Qt 6 Widgets, CMake. Skeleton copied from ~/builds/games/solitare (theme core, CLI, watcher, in-window modals with blur).

## Users

Keyboard-first desktop users on Wayland who want a quick Snake break. Anyone can download it, then theme it from the CLI.

## Product Purpose

"Snake": a small, portable, standalone Snake client, a clone of Faeric/omasnake rules (20x16 grid, walls kill, +1 per food, tick 165ms down to 68ms at 4ms per point, saved high score).

## Positioning

Memory-light native binary, fully themeable through `snake theme ...` and a live-reloaded theme.json. No web runtime.

## Capabilities and Constraints

- Controls: arrows/WASD/HJKL, Space/Enter/P start+pause, R restart.
- Theme tokens saved in JSON, CLI edits the file and the open window reloads.
- Colors: #RRGGBB, alpha-first #AARRGGBB, Qt.rgba(...). Opacity preserved when the compositor supports it.
- Only themeable (recolorable) icons inside the app. The launcher icon is fixed, colored to match the app content.
- Popups are modals inside the window with blurred backdrop.
- Docs are shell-agnostic: standalone and portable.

## Brand Commitments

Name is exactly "Snake". No gradients anywhere. Visual direction: lofi + TUI, contrast first.

## Evidence on Hand

Reference behavior read from the omasnake README and BarWidget.qml; not run.

## Product Principles

1. Contrast over atmosphere: every glyph and cell reads at a glance.
2. The palette is the user's; the app never invents colors outside the theme tokens.
3. Quiet, keyboard-first, instant to start.
