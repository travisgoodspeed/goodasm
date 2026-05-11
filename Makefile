# Top-level Makefile for GoodASM
# Forwards targets to tests/Makefile

.PHONY: all clean selftests fuzz ensure_task unidasm qt5deps yara_x tests-all build

# Default: configure & build with CMake (creates build/ if missing)
CURDIR:=$(shell pwd)
BUILD_PATHS:=$(CURDIR)/build:$(CURDIR)/build/Debug:$(CURDIR)/build/Release:$(CURDIR)/scripts
export PATH:=$(BUILD_PATHS):$(PATH)

all: build tests-all

build:
	@mkdir -p build
	@cmake -S . -B build || true
	@cmake --build build
# Preserve old behavior to forward to tests when explicitly requested
tests-all: yara_x nasm qt5deps unidasm
	$(MAKE) -C tests all

clean:
	$(MAKE) -C tests clean

ifeq ($(OS),Windows_NT)
	@echo "Removing unidasm.exe from %USERPROFILE%\\bin if present";
	@cmd /C "if exist %USERPROFILE%\\bin\\unidasm.exe del %USERPROFILE%\\bin\\unidasm.exe" || true
else
	@echo "Removing local unidasm if present";
	@rm -f "$$HOME/.local/bin/unidasm" || true;
	@# If a system-wide copy exists on macOS/linux, remove if writable or show sudo hint
	@if [ -f /usr/local/bin/unidasm ]; then \
		if [ -w /usr/local/bin ]; then \
			rm -f /usr/local/bin/unidasm && echo "removed /usr/local/bin/unidasm"; \
		else \
			echo "/usr/local/bin/unidasm exists; to remove run: sudo rm /usr/local/bin/unidasm"; \
		fi; \
	fi;
endif

selftests:
	$(MAKE) -C tests selftests

fuzz:
	$(MAKE) -C tests fuzz

ensure_task:
ifeq ($(OS),Windows_NT)
	cmd /C scripts\\ensure_task.cmd
else
	bash ./scripts/ensure_task.sh
endif

qt5deps:
	@echo "checking for Qt5 (qmake/pk-config) on PATH...";
	@if command -v qmake-qt5 >/dev/null 2>&1 || command -v qmake >/dev/null 2>&1 || (command -v pkg-config >/dev/null 2>&1 && pkg-config --exists Qt5Core); then \
		echo "Qt5 appears to be installed: $$(command -v qmake-qt5 || command -v qmake || echo pkg-config)"; \
	else \
		if command -v apt-get >/dev/null 2>&1; then \
			sudo apt-get update && sudo apt-get install -y qtbase5-dev qt5-qmake qtbase5-dev-tools pkg-config libsdl2-ttf-dev; \
		elif command -v brew >/dev/null 2>&1; then \
			brew install qt@5 pkg-config sdl2_ttf; \
		else \
			echo "Please install Qt5 dev tools, pkg-config, and SDL2_ttf manually."; \
		fi; \
	fi


nasm:
	@echo "checking for nasm on PATH...";
	@if command -v nasm >/dev/null 2>&1; then \
		echo "nasm is already on PATH at: $$(command -v nasm)"; \
	else \
		if command -v apt-get >/dev/null 2>&1; then \
			sudo apt-get update && sudo apt-get install -y nasm; \
		elif command -v brew >/dev/null 2>&1; then \
			brew install nasm; \
		else \
			echo "Please install nasm manually"; \
		fi; \
	fi


yara_x:
	@echo "checking for yr on PATH...";
	@if command -v yr >/dev/null 2>&1; then \
		echo "yr is already on PATH at: $$(command -v yr)"; \
	else \
		if [ ! -d yara-x ]; then git clone --depth 1 https://github.com/VirusTotal/yara-x.git; fi; \
		cargo install --path yara-x/cli --root $$HOME/.local; \
	fi

# unidasm: qt5deps
# 	@if [ ! -d mame ]; then git clone --depth 1 https://github.com/mamedev/mame.git; fi
# 	$(MAKE) -C mame TOOLS=1 generate target=generate
# 	$(MAKE) -C mame TOOLS=1 unidasm target=unidasm
# 	@if [ -f mame/bin/unidasm ]; then \
# 		mkdir -p $$HOME/.local/bin; \
# 		cp mame/bin/unidasm $$HOME/.local/bin/unidasm; \
# 	elif [ -f mame/unidasm ]; then \
# 		mkdir -p $$HOME/.local/bin; \
# 		cp mame/unidasm $$HOME/.local/bin/unidasm; \
# 	else \
# 		echo 'unidasm binary not found!'; exit 1; \
# 	fi

# .PHONY: unidasm
unidasm:
	@if [ ! -d mame ]; then git clone --depth 1 https://github.com/mamedev/mame.git; fi
ifeq ($(OS),Windows_NT)
	@echo "Building unidasm on Windows...";
	@cmd /C scripts\\build-unidasm.cmd
else
	@echo "Building unidasm on Unix-like system...";
	@if sh scripts/build-unidasm.sh; then \
		if [ -f "$$HOME/.local/bin/unidasm" ]; then \
			echo "unidasm built and copied to $$HOME/.local/bin/unidasm"; \
			echo "To install system-wide to /usr/local/bin run:"; \
			echo "  sudo cp \"$$HOME/.local/bin/unidasm\" /usr/local/bin/unidasm && sudo chmod 755 /usr/local/bin/unidasm"; \
		else \
			echo "build script reported success but unidasm not found at $$HOME/.local/bin/unidasm"; exit 1; \
		fi; \
	else \
		echo "build-unidasm.sh failed; see its output above for details"; exit 1; \
	fi
endif
