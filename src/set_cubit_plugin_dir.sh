#!/usr/bin/env bash

# Set CUBIT_PLUGIN_DIR to the directory containing this script, grant the
# current user write access to it, and save the variable for future shells.
set -u

plugin_dir="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)" || {
    printf 'Failed to resolve the plugin directory.\n' >&2
    exit 1
}
plugin_user="$(id -un 2>/dev/null)" || {
    printf 'Failed to identify the current user.\n' >&2
    exit 1
}

if [[ -z "$plugin_user" ]]; then
    printf 'Failed to identify the current user.\n' >&2
    exit 1
fi

# Match the Windows script's intent: make the current user's files writable
# and directories traversable, without changing permissions for other users.
if ! chmod -R u+rwX -- "$plugin_dir"; then
    printf 'Failed to grant write access to "%s".\n' "$plugin_dir" >&2
    printf 'Run this script as the owner of the files, or with sudo if needed.\n' >&2
    exit 1
fi

case "$(basename -- "${SHELL:-/bin/bash}")" in
    zsh) startup_file="$HOME/.zshrc" ;;
    bash) startup_file="$HOME/.bashrc" ;;
    *) startup_file="$HOME/.profile" ;;
esac

if [[ ! -w "$HOME" ]] || { [[ -e "$startup_file" ]] && [[ ! -w "$startup_file" ]]; }; then
    printf 'Failed to save CUBIT_PLUGIN_DIR in "%s" (insufficient user permissions).\n' "$startup_file" >&2
    exit 1
fi

mkdir -p -- "$(dirname -- "$startup_file")" || {
    printf 'Failed to prepare startup file "%s".\n' "$startup_file" >&2
    exit 1
}
touch -- "$startup_file" || {
    printf 'Failed to save CUBIT_PLUGIN_DIR in "%s".\n' "$startup_file" >&2
    exit 1
}

# Replace a previous entry from this script so rerunning it is safe.
tmp_file="$(mktemp "${startup_file}.XXXXXX")" || {
    printf 'Failed to update "%s".\n' "$startup_file" >&2
    exit 1
}
if ! awk '!/^# Set by set_cubit_plugin_dir\.sh$/ && !/^export CUBIT_PLUGIN_DIR=/' "$startup_file" > "$tmp_file"; then
    rm -f -- "$tmp_file"
    printf 'Failed to update "%s".\n' "$startup_file" >&2
    exit 1
fi
{
    printf '\n# Set by set_cubit_plugin_dir.sh\n'
    printf 'export CUBIT_PLUGIN_DIR=%q\n' "$plugin_dir"
} >> "$tmp_file" || {
    rm -f -- "$tmp_file"
    printf 'Failed to update "%s".\n' "$startup_file" >&2
    exit 1
}
chmod --reference="$startup_file" "$tmp_file" 2>/dev/null || true
if ! mv -- "$tmp_file" "$startup_file"; then
    rm -f -- "$tmp_file"
    printf 'Failed to save CUBIT_PLUGIN_DIR in "%s".\n' "$startup_file" >&2
    exit 1
fi

export CUBIT_PLUGIN_DIR="$plugin_dir"
printf 'CUBIT_PLUGIN_DIR=%s\n' "$CUBIT_PLUGIN_DIR"
printf 'Saved in %s. Restart Cubit (and open a new shell) to use the new value.\n' "$startup_file"
