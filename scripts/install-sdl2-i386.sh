#!/usr/bin/env bash
set -euo pipefail
PLATFORM=$(uname -s 2>/dev/null || echo Unknown)
echo "[INFO] Checking for 32-bit SDL2 development libraries..."

# Ensure i386 architecture is enabled
dpkg --print-foreign-architectures | grep -q i386 || {
  echo "[INFO] Enabling i386 architecture..."
  sudo dpkg --add-architecture i386
  sudo apt-get update
}

echo "[INFO] Installing 32-bit SDL2 development package..."
sudo apt-get install -y libsdl2-dev:i386

# Check for 32-bit SDL2 library presence
if ls /usr/lib/i386-linux-gnu/libSDL2*.so* 1>/dev/null 2>&1; then
  echo "[SUCCESS] 32-bit SDL2 development libraries are installed."
else
  echo "[ERROR] 32-bit SDL2 development libraries are still missing!"
  exit 1
fi


  # Ensure multilib support for 32/64-bit C++ headers if using apt-get
  if [ "$PLATFORM" = "Linux" ] && command -v apt-get >/dev/null 2>&1; then
    echo "Checking for gcc-multilib and g++-multilib..."
    if ! dpkg -s gcc-multilib g++-multilib >/dev/null 2>&1; then
      echo "Installing gcc-multilib and g++-multilib via apt-get (sudo required)..."
      sudo apt-get update && sudo apt-get install -y gcc-multilib g++-multilib
    else
      echo "gcc-multilib and g++-multilib already installed."
    fi
  fi