#!/usr/bin/env bash
# Cross-platform script to ensure Go Task is installed
# Prefers npm, then Homebrew, apt, dnf, winget, or snap
set -e

# Check if task is already installed
if command -v task >/dev/null 2>&1; then
  echo "[ensure_task] Task is already installed."
  exit 0
fi

# Prefer npm if available
if command -v npm >/dev/null 2>&1; then
  echo "[ensure_task] Installing Task via npm..."
  npm install -g @go-task/cli
  exit $?
fi

# macOS or Linux with Homebrew
if command -v brew >/dev/null 2>&1; then
  echo "[ensure_task] Installing Task via Homebrew..."
  brew install go-task/tap/go-task || brew install go-task
  exit $?
fi

# Ubuntu/Debian/Mint (apt)
if command -v apt >/dev/null 2>&1; then
  echo "[ensure_task] Installing Task via apt..."
  curl -1sLf 'https://dl.cloudsmith.io/public/task/task/setup.deb.sh' | sudo -E bash
  sudo apt install -y task
  exit $?
fi

# Fedora/CentOS (dnf)
if command -v dnf >/dev/null 2>&1; then
  echo "[ensure_task] Installing Task via dnf..."
  curl -1sLf 'https://dl.cloudsmith.io/public/task/task/setup.rpm.sh' | sudo -E bash
  sudo dnf install -y task
  exit $?
fi

# Snap (Linux/macOS)
if command -v snap >/dev/null 2>&1; then
  echo "[ensure_task] Installing Task via snap..."
  sudo snap install task --classic
  exit $?
fi

# Windows (winget)
if command -v winget >/dev/null 2>&1; then
  echo "[ensure_task] Installing Task via winget..."
  winget install -e --id Task.Task
  exit $?
fi

# If we reach here, we couldn't install Task
>&2 echo "[ensure_task] ERROR: Could not find a supported package manager to install Task."
exit 1
