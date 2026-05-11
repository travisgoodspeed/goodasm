#!/usr/bin/env bash

# Robust: don't exit on error, unset variable, or pipefail
set +e


# Always use script's directory as base
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR/.." || exit 0
echo "[DEBUG] Script directory: $SCRIPT_DIR"
echo "[DEBUG] Current working directory: $(pwd)"
echo "[DEBUG] PLATFORM: $(uname -s 2>/dev/null || echo Unknown)"

# Detect platform early (used when invoking generator)
PLATFORM=$(uname -s 2>/dev/null || echo Unknown)

 # If mame/ is missing, clone shallow copy (do this before searching)
 if [ ! -d mame ]; then
   echo "mame/ not found — cloning shallow repository (this may take a while)";
   git clone --depth 1 https://github.com/mamedev/mame.git || true
 fi


# Find generated gmake dir, relative to script
G=""
if [ "$PLATFORM" = "Darwin" ]; then
  for d in mame/build/projects/sdl/mame/gmake-osx-clang mame/build/projects/sdl/mame/gmake-osx; do
    if [ -d "$d" ]; then G="$d"; break; fi
  done
elif [ "$PLATFORM" = "Linux" ]; then
  for d in mame/build/projects/sdl/mame/gmake-linux-clang mame/build/projects/sdl/mame/gmake-linux; do
    if [ -d "$d" ]; then G="$d"; break; fi
  done
fi

# Fallback: try to generate the gmake tree if not found
if [ -z "$G" ]; then
  echo "[INFO] No gmake dir found, attempting to generate..."
  if [ "$PLATFORM" = "Darwin" ]; then
    (cd mame && make TOOLS=1 build/projects/sdl/mame/gmake-osx-clang/Makefile) || true
    for d in mame/build/projects/sdl/mame/gmake-osx-clang mame/build/projects/sdl/mame/gmake-osx; do
      if [ -d "$d" ]; then G="$d"; break; fi
    done
  elif [ "$PLATFORM" = "Linux" ]; then
    (cd mame && make TOOLS=1 build/projects/sdl/mame/gmake-linux-clang/Makefile) || true
    for d in mame/build/projects/sdl/mame/gmake-linux-clang mame/build/projects/sdl/mame/gmake-linux; do
      if [ -d "$d" ]; then G="$d"; break; fi
    done
  fi
fi

# Last resort: pick any gmake dir
if [ -z "$G" ]; then
  G=$(ls -d mame/build/projects/*/*gmake* 2>/dev/null | head -n1 || true)
fi

echo "[DEBUG] G (make dir): $G"
if [ -z "$G" ]; then
  echo "[WARN] G (make dir) is still empty. Will try to build anyway with generic defaults."
  G="mame/build/projects/sdl/mame/gmake-linux"
fi
if [ ! -f "$G/Makefile" ]; then
  echo "[WARN] $G/Makefile does not exist. Will try to build anyway."
fi



PLATFORM=$(uname -s 2>/dev/null || echo Unknown)
BREW_PREFIX=""
CPPFLAGS=""
LDFLAGS=""
LIBS=""

if [ "$PLATFORM" = "Darwin" ]; then
  BREW_PREFIX=$(brew --prefix 2>/dev/null || echo /usr/local)
  CPPFLAGS="-I${BREW_PREFIX}/include -I${BREW_PREFIX}/include/SDL2"
  LDFLAGS="-L${BREW_PREFIX}/lib"
  LIBS="-lpthread -L${BREW_PREFIX}/lib -lSDL2 -framework Cocoa"
  echo "building unidasm from $G with brewed SDL2 (macOS)"

  # Patch generated makefiles to avoid expecting SDL2 as an Xcode framework.
  # Replace occurrences of '-framework SDL2' with brewed linker flags and remove framework-specific rpaths.
  BREW_LIBS="-L${BREW_PREFIX}/lib -lSDL2"
  for f in "$G"/*.make "$G"/Makefile; do
    if [ -f "$f" ]; then
      # macOS sed supports '-i.bak' to write a backup file
      sed -i.bak -e "s|-framework SDL2|${BREW_LIBS}|g" -e "s|-F/Library/Frameworks/||g" -e "s|-rpath /Library/Frameworks/||g" "$f" || true
    fi
  done

else
  # Linux / other Unix
  echo "building unidasm from $G (non-macOS)"

  # Prefer pkg-config for SDL2 if available
  if command -v pkg-config >/dev/null 2>&1 && pkg-config --exists sdl2; then
    CPPFLAGS="$(pkg-config --cflags sdl2 2>/dev/null || true)"
    LDFLAGS="$(pkg-config --libs-only-L sdl2 2>/dev/null || true)"
    # libs-only-l returns -lSDL2; keep full libs for LIBS
    LIBS="$(pkg-config --libs sdl2 2>/dev/null || true) -lpthread"
  else
    # Fall back to /usr/local
    BREW_PREFIX=$(brew --prefix 2>/dev/null || echo /usr/local)
    CPPFLAGS="-I${BREW_PREFIX}/include"
    LDFLAGS="-L${BREW_PREFIX}/lib"
    LIBS="-lpthread -L${BREW_PREFIX}/lib -lSDL2"
  fi

  # Add architecture-specific C++ include path if needed (for c++config.h)
  GCC_VER=$(g++ -dumpversion | cut -d. -f1)
  ARCH=$(g++ -dumpmachine 2>/dev/null || echo x86_64-linux-gnu)
  STDCPP_ARCH_PATH="/usr/include/c++/$GCC_VER/$ARCH"
  if [ -d $STDCPP_ARCH_PATH ]; then
    CPPFLAGS="$CPPFLAGS -I$STDCPP_ARCH_PATH"
  fi

  # Add multiarch SDL2 include paths if needed (for _real_SDL_config.h)
  if [ -d /usr/include/x86_64-linux-gnu/SDL2 ]; then
    CPPFLAGS="$CPPFLAGS -I/usr/include/x86_64-linux-gnu/SDL2 -I/usr/include/x86_64-linux-gnu"
  fi
fi

echo "CPPFLAGS=${CPPFLAGS} LDFLAGS=${LDFLAGS} LIBS=${LIBS}"



echo "[DEBUG] Running: env CPPFLAGS=\"$CPPFLAGS\" LDFLAGS=\"$LDFLAGS\" LIBS=\"$LIBS\" make -C $G unidasm"
env CPPFLAGS="$CPPFLAGS" LDFLAGS="$LDFLAGS" LIBS="$LIBS" make -C "$G" unidasm || echo "[WARN] make failed, continuing anyway."

# Try common result locations and copy to ~/.local/bin
DEST="$HOME/.local/bin"
mkdir -p "$DEST"
UNIDASM_CANDIDATES=(
  "mame/build/generated/unidasm/unidasm"
  "mame/build/generated/unidasm"
  "mame/unidasm"
  "$G/../../../../unidasm"
  "$G/../../../../mame/unidasm"
  "$G/../../../../mame/bin/unidasm"
)
FOUND=0
for candidate in "${UNIDASM_CANDIDATES[@]}"; do
  if [ -f "$candidate" ]; then
    echo "[INFO] Found unidasm binary at: $candidate"
    cp "$candidate" "$DEST/unidasm"
    FOUND=1
    break
  fi
done
if [ "$FOUND" -eq 0 ]; then
  echo "[WARN] unidasm binary not found after build, but script will not abort."
fi
