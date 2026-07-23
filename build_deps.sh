#!/bin/bash

set -e

# ------------------------------------------------------------------------------
# Colors
# ------------------------------------------------------------------------------

if [ -t 1 ]; then
    C_RESET="\033[0m"
    C_BOLD="\033[1m"
    C_DIM="\033[2m"
    C_RED="\033[31m"
    C_GREEN="\033[32m"
    C_YELLOW="\033[33m"
    C_BLUE="\033[34m"
    C_CYAN="\033[36m"
else
    C_RESET="" C_BOLD="" C_DIM="" C_RED="" C_GREEN="" C_YELLOW="" C_BLUE="" C_CYAN=""
fi

# ------------------------------------------------------------------------------
# Output helpers
# ------------------------------------------------------------------------------

print_header() {
    echo ""
    echo -e "${C_BOLD}${C_BLUE}:: ${C_RESET}${C_BOLD}$1${C_RESET}"
}

print_step() {
    echo -e "${C_DIM}   $1${C_RESET}"
}

print_success() {
    echo -e "${C_GREEN}   $1${C_RESET}"
}

print_error() {
    echo -e "${C_RED}error:${C_RESET} $1" >&2
}

print_warning() {
    echo -e "${C_YELLOW}warning:${C_RESET} $1"
}

print_info() {
    echo -e "   ${C_DIM}$1${C_RESET}"
}

# ------------------------------------------------------------------------------
# Dependencies
# ------------------------------------------------------------------------------

build_deps() {
    print_header "Building Dependencies"

    local SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    local LIBS_DIR="$SCRIPT_DIR/libs"

    # raylib
    if [ -d "$LIBS_DIR/raylib" ]; then
        print_step "Building raylib..."
        mkdir -p "$LIBS_DIR/raylib/build"
        cmake -S "$LIBS_DIR/raylib" -B "$LIBS_DIR/raylib/build" \
            -DCUSTOMIZE_BUILD=ON \
            -DSUPPORT_FILEFORMAT_JPG=ON \
            -DBUILD_EXAMPLES=OFF \
            -DCMAKE_BUILD_TYPE=Release \
            > /dev/null
        make -C "$LIBS_DIR/raylib/build" -j$(nproc) > /dev/null
        print_success "raylib built"
    else
        print_error "raylib not found at $LIBS_DIR/raylib"
        exit 1
    fi

    echo ""
    print_success "All dependencies built successfully"
}

build_deps

