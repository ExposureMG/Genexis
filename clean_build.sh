#!/usr/bin/env bash
# Delete the build directory and rebuild from scratch.
# Takes the same environment variables and arguments as build.sh.
set -euo pipefail

cd "$(dirname "${BASH_SOURCE[0]}")"

BUILD_DIR=${BUILD_DIR:-build}

die() {
  echo "clean_build.sh: $*" >&2
  exit 1
}

[[ -n "$BUILD_DIR" ]] || die "BUILD_DIR is empty"

if [[ -e "$BUILD_DIR" ]]; then
  [[ -d "$BUILD_DIR" ]] || die "'$BUILD_DIR' is not a directory"
  repo_root=$(pwd -P)
  target=$(cd -- "$BUILD_DIR" && pwd -P)
  case "$target" in
    / | "$HOME") die "refusing to delete '$target'" ;;
  esac
  case "$repo_root/" in
    "$target"/*) die "refusing to delete '$target': it contains the sources" ;;
  esac
  if [[ -n "$(ls -A -- "$target")" && ! -f "$target/CMakeCache.txt" ]]; then
    die "refusing to delete '$target': no CMakeCache.txt, not a build directory"
  fi
  rm -rf -- "$target"
fi

exec ./build.sh "$@"
