#!/usr/bin/env bash
# macOS: real input-buffer integration + preferences, with memory/UB checking.
set -euo pipefail
cd "$(dirname "$0")/.."
TASK_TMP=$(mktemp -d)
trap 'rm -rf "$TASK_TMP"' EXIT
TASK_INCLUDES=()
for TASK_INCLUDE in BungieFrameWork/BFW_Source/* BungieFrameWork/BFW_Source/BFW_WindowManager/* OniProj/OniGameSource/*; do
    [ -d "$TASK_INCLUDE" ] && TASK_INCLUDES+=("-I$TASK_INCLUDE")
done
for TASK_TEST in test_mouse_sensitivity test_mouse_sensitivity_ui test_mouse_sensitivity_poll; do
cc "${TASK_INCLUDES[@]}" -std=gnu11 -O1 -g -fsanitize=address,undefined -Wno-multichar \
    -Werror=incompatible-pointer-types -Wno-incompatible-function-pointer-types \
    -DUUmSDL=1 -DSHIPPING_VERSION=1 -DNDEBUG -isystem /opt/homebrew/include \
    -IBungieFrameWork/BFW_Source/BFW_Motoko/Engines/DrawEngine/3DFX_Glide \
    -IBungieFrameWork/BFW_Source/BFW_Motoko/Manager \
    -IBungieFrameWork/BFW_Headers -IBungieFrameWork/BFW_Source/BFW_MathLib \
    -IBungieFrameWork/BFW_Source/BFW_LocalInput \
    -IBungieFrameWork/BFW_Source/BFW_LocalInput/Platform_SDL \
    -IBungieFrameWork/BFW_Source/BFW_SoundSystem2 \
    -IOniProj/OniGameSource -IOniProj/OniGameSource/Oni_AI \
    -IOniProj/OniGameSource/Oni_Object \
    "tests/$TASK_TEST.c" OniProj/OniGameSource/Oni_MouseSensitivity.c \
    -Wl,-dead_strip -o "$TASK_TMP/$TASK_TEST"
    "$TASK_TMP/$TASK_TEST"
done
