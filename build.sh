#!/bin/bash

CC=gcc
INC="-I./include -I./libs/raylib/src"
DEPS_BASE="-L./libs/raylib/build/raylib -lraylib -lm -lX11"
DEPS="$DEPS_BASE"
LD_FLAGS=""

SRC_DIR="./src"
BUILD_DIR="./build"
OUTPUT_BIN="match-tudo"
TARGET="$BUILD_DIR/$OUTPUT_BIN"

GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m'

if [ "$1" = "--hot-build" ]; then
  FILE="$2"
  OUT="$3"
  $CC -shared -fPIC -x c -o "$OUT" "$FILE" \
      $INC -DHOT_RELOAD=1 -DMODULE_BUILD=1 -DDEBUG \
      -Wno-pragma-once-outside-header
  exit $?
fi

HOT_RELOAD=false
RELEASE=false
APP_DEBUG=false
APP_LOG_LEVEL=0
APP_UNDECORATED=false
APP_TRANSPARENT=false
APP_TOPMOST=false
APP_FULL_SCREEN=false
APP_RELOAD_ON_SAVE=false
WINDOW_MONITOR=""
WINDOW_WIDTH=""
WINDOW_HEIGHT=""

show_help() {
  cat << EOF
Usage: ./build.sh [options]

Options:
  --help                    Show this help message
  --hot-reload[=true|false] Enable hot-reload mode (default: true if no value)
  -hot                      Shorthand for --hot-reload=true
  -m, --monitor <idx>       Set APP_WINDOW_MONITOR
  -w, --width <px>          Set APP_WINDOW_WIDTH
  -h, --height <px>         Set APP_WINDOW_HEIGHT
  --debug, -d               Enable debug defines (-DDEBUG -DDEBUG_MEMORY_USAGE=1)
  --log-level, -ll          log level (0 ... 5 : 0 = NONE 5 = ALL)
  --behavior-tre, -bt       generate behavior tree AOT
  --undecorated, -u         Set APP_WINDOW_UNDECORATED=1
  --transparent, -tt        Set DAPP_WINDOW_TRANSPARENT=1
  --top, -t                 Set APP_WINDOW_TOPMOST=1
  --full-screen, -fs        Set APP_FULL_SCREEN=1
  --reload-on-save, -rs     Set HOT_RELOAD_UPDATE_ON_SAVE=1
  --release                 Build for release (-O3, no debug, no hot-reload)
EOF
  exit 0
}

while [ $# -gt 0 ]; do
  case "$1" in
    --help)
      show_help
      ;;
    --hot-reload=*)
      case "${1#*=}" in
        true|1|yes) HOT_RELOAD=true ;;
        false|0|no) HOT_RELOAD=false ;;
        *) echo "error: invalid value for --hot-reload" >&2; exit 1 ;;
      esac
      ;;
    --hot-reload)
      HOT_RELOAD=true
      ;;
    -hot)
      HOT_RELOAD=true
      ;;
    --monitor|-m)
      WINDOW_MONITOR="$2"; shift
      ;;
    --width|-w)
      WINDOW_WIDTH="$2"; shift
      ;;
    --width=*|-w=*)
      WINDOW_WIDTH="${1#*=}"
      ;;
    --height|-h)
      WINDOW_HEIGHT="$2"; shift
      ;;
    --height=*|-h=*)
      WINDOW_HEIGHT="${1#*=}"
      ;;
    --debug|-d)
      APP_DEBUG=true
      ;;
    --log-level|-ll)
      APP_LOG_LEVEL="$2"; shift
      ;;
    --log-level=*|-ll=*)
      APP_LOG_LEVEL="${1#*=}"
      ;;
    --undecorated|-u)
      APP_UNDECORATED=true
      ;;
    --transparent|-tt)
      APP_TRANSPARENT=true
      ;;
    --top|-t)
      APP_TOPMOST=true
      ;;
    --full-screen|-fs)
      APP_FULL_SCREEN=true
      ;;
    --reload-on-save|-rs)
      APP_RELOAD_ON_SAVE=true
      ;;
    --release)
      RELEASE=true
      ;;
    *)
      echo "error: unknown option: $1" >&2
      echo "use --help for usage" >&2
      exit 1
      ;;
  esac
  shift
done

if [ "$RELEASE" = true ]; then
  CFLAGS="-Wall -Wextra -std=c11 -flto=auto -O3 -pedantic"
  DEFS="-DRELEASE -DAPP_PACKAGE_RESOURCE=1"
  DEPS="$DEPS_BASE"
  LD_FLAGS=""
elif [ "$HOT_RELOAD" = true ]; then
  CFLAGS="-Wall -Wextra -std=c11 -g -O0 -pedantic"
  LD_FLAGS="-rdynamic"
  DEPS="-L./libs/raylib/build/raylib -lraylib -ldl -lm -lX11"
  DEFS="-DHOT_RELOAD=1"
else
  CFLAGS="-Wall -Wextra -std=c11 -flto=auto -O0 -g -fsanitize=address -pedantic -Wfatal-errors"
  DEPS="$DEPS_BASE"
  LD_FLAGS=""
  DEFS=""
fi

if [ "$APP_DEBUG" = true ] && [ "$RELEASE" = false ]; then
  DEFS+=" -DDEBUG -DDEBUG_MEMORY_USAGE=1"
fi

[ -n "$APP_LOG_LEVEL" ] && DEFS+=" -DLOG_LEVEL=$APP_LOG_LEVEL"
[ "$APP_UNDECORATED" = true ] && DEFS+=" -DAPP_WINDOW_UNDECORATED=1"
[ "$APP_TRANSPARENT" = true ] && DEFS+=" -DAPP_WINDOW_TRANSPARENT=1"
[ "$APP_TOPMOST" = true ] && DEFS+=" -DAPP_WINDOW_TOPMOST=1"
[ "$APP_FULL_SCREEN" = true ] && DEFS+=" -DAPP_FULL_SCREEN=1"
[ "$APP_RELOAD_ON_SAVE" = true ] && DEFS+=" -DHOT_RELOAD_UPDATE_ON_SAVE=1"
[ -n "$WINDOW_MONITOR" ] && DEFS+=" -DAPP_WINDOW_MONITOR=$WINDOW_MONITOR"
[ -n "$WINDOW_WIDTH" ] && DEFS+=" -DAPP_WINDOW_WIDTH=$WINDOW_WIDTH"
[ -n "$WINDOW_HEIGHT" ] && DEFS+=" -DAPP_WINDOW_HEIGHT=$WINDOW_HEIGHT"

echo "Starting build process..."

mkdir -p "$BUILD_DIR"
rm -f "$BUILD_DIR"/*.o

SOURCES=$(find "$SRC_DIR" -name "*.c")

echo "Compiling..."
for file in $SOURCES; do
  filename=$(basename "$file")
  echo "Compiling: $filename"
  $CC $CFLAGS $INC $DEFS -c "$file" -o "$BUILD_DIR/${filename%.c}.o"
  if [ $? -gt 0 ]; then
    echo -e "${RED}compilation failed!${NC}"
    exit 1
  fi
done

echo "Linking..."
$CC "$BUILD_DIR"/*.o -o "$TARGET" $INC $DEFS $DEPS $LD_FLAGS -lm

if [ $? -eq 0 ]; then
  echo -e "${GREEN}Build successful! Binary created at: $TARGET${NC}"
else
  echo -e "${RED}Build failed!${NC}"
  exit 1
fi

if [ "$RELEASE" = true ]; then
  echo "Packing..."
  $CC $CFLAGS $INC $DEFS tools/mkpackage.c $DEPS -o build/mkpackage
  build/mkpackage
  cp resources/package/package.data build/package.data
fi
