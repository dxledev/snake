#!/usr/bin/env bash
set -euo pipefail
project_directory="$(cd -- "${BASH_SOURCE[0]%/*}/.." && pwd)"
build_directory="$project_directory/build"
install_prefix="$HOME/.local"
config_directory="${XDG_CONFIG_HOME:-$HOME/.config}/snake"
apps_list="${XDG_CONFIG_HOME:-$HOME/.config}/apps.list"
follow_path=""
dry_run=0
usage() {
    printf '%s\n' 'Usage: install-desktop.sh [--dry-run] [--build-dir DIR] [--prefix DIR]' \
        '       [--config-dir DIR] [--apps-list FILE] [--follow PALETTE_DIRECTORY]'
}
fail() { printf 'install-desktop: %s\n' "$*" >&2; exit 1; }
parse_options() {
    while (( $# )); do
        case "$1" in
            --dry-run) dry_run=1; shift ;;
            --help|-h) usage; exit 0 ;;
            --build-dir|--prefix|--config-dir|--apps-list|--follow)
                (( $# >= 2 )) || fail "Missing value for $1"
                [[ "$2" == /* && "$2" != / && "$2" != *$'\n'* && "$2" != *'|'* && "$2" != *'"'* && "$2" != *'%'* && "$2" != *'\\'* && "$2" != *'`'* && "$2" != *'$'* ]] || fail "Expected a safe absolute path for $1"
                case "$1" in
                    --build-dir) build_directory="${2%/}" ;;
                    --prefix) install_prefix="${2%/}" ;;
                    --config-dir) config_directory="${2%/}" ;;
                    --apps-list) apps_list="$2" ;;
                    --follow) follow_path="${2%/}" ;;
                esac
                shift 2 ;;
            *) fail "Unknown option: $1" ;;
        esac
    done
}
validate() {
    [[ -x "$build_directory/snake" ]] || fail "Build first: $build_directory/snake"
    [[ ! -L "$apps_list" || -f "$apps_list" ]] || fail 'Invalid app list'
    [[ ! -L "$install_prefix/bin/snake" ]] || fail 'Executable path is a symlink'
    local output
    for output in "$install_prefix/bin/applications/io.github.snake.desktop" "$install_prefix/share/applications/io.github.snake.desktop" "$install_prefix/share/icons/hicolor/scalable/apps/io.github.snake.svg"; do
        [[ ! -L "$output" && ( ! -e "$output" || -f "$output" ) ]] || fail "Refusing to replace an unexpected output: $output"
    done
    if [[ -n "$follow_path" ]]; then
        "$build_directory/snake" --config-dir "$config_directory" theme follow symlink "$follow_path" --dry-run >/dev/null
    fi
}
install_files() {
    local temporary desktop="$install_prefix/bin/applications/io.github.snake.desktop"
    /usr/bin/mkdir -p -- "$install_prefix/bin" "${desktop%/*}" "$install_prefix/share/applications"
    temporary="$(/usr/bin/mktemp "$install_prefix/bin/.snake.XXXXXX")"
    /usr/bin/install -m755 -- "$build_directory/snake" "$temporary"
    /usr/bin/mv -T -- "$temporary" "$install_prefix/bin/snake"
    /usr/bin/install -Dm644 -- "$project_directory/assets/io.github.snake.svg" "$install_prefix/share/icons/hicolor/scalable/apps/io.github.snake.svg"
    /usr/bin/awk -v executable="$install_prefix/bin/snake" -v config="$config_directory" \
        '/^Exec=/ {print "Exec=\"" executable "\" --config-dir \"" config "\"";next} {print}' "$project_directory/assets/io.github.snake.desktop" > "$desktop"
    /usr/bin/install -m644 -- "$desktop" "$install_prefix/share/applications/io.github.snake.desktop"
    if [[ -x /usr/bin/update-desktop-database ]]; then /usr/bin/update-desktop-database "$install_prefix/share/applications"; fi
}
register_app() {
    local target="$apps_list" temporary
    if [[ -L "$target" ]]; then target="$(/usr/bin/readlink -f -- "$target")"; fi
    /usr/bin/mkdir -p -- "${target%/*}"
    if [[ -f "$target" ]] && /usr/bin/awk -F '|' '$1=="io.github.snake.desktop" {found=1} END {exit !found}' "$target"; then return; fi
    temporary="$(/usr/bin/mktemp "${target%/*}/.snake-apps.XXXXXX")"
    if [[ -f "$target" ]]; then /usr/bin/cp -p -- "$target" "$target.snake-backup.$(/usr/bin/date +%Y%m%dT%H%M%S).$$"; /usr/bin/awk '{print}' "$target" > "$temporary"; /usr/bin/chmod --reference="$target" "$temporary"; fi
    printf 'io.github.snake.desktop|%s/share/icons/hicolor/scalable/apps/io.github.snake.svg|Snake\n' "$install_prefix" >> "$temporary"
    /usr/bin/mv -T -- "$temporary" "$target"
}
main() {
    parse_options "$@";validate
    if (( dry_run )); then
        printf 'Install: %s/bin/snake\nDesktop: %s/bin/applications/io.github.snake.desktop\nApp list: %s\nPalette: %s\n' "$install_prefix" "$install_prefix" "$apps_list" "${follow_path:-saved palette or default}"
        return
    fi
    install_files;register_app
    if [[ -n "$follow_path" ]]; then "$install_prefix/bin/snake" --config-dir "$config_directory" theme follow symlink "$follow_path" >/dev/null; fi
    printf 'Installed Snake.\n'
}
main "$@"
