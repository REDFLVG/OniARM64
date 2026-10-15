#!/usr/bin/env bash
# macOS standalone test of the real BFW_TextSystem.c; run from repository root.
set -euo pipefail
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT
cc -std=gnu11 -O1 -g -fsanitize=address,undefined -Wno-multichar \
    -DUUmSDL=1 -DSHIPPING_VERSION=1 -DNDEBUG -isystem /opt/homebrew/include \
    -IBungieFrameWork/BFW_Headers \
    -IBungieFrameWork/BFW_Source/BFW_TextSystem \
    -IBungieFrameWork/BFW_Source/BFW_MathLib \
    -IBungieFrameWork/BFW_Source/BFW_WindowManager \
    -IBungieFrameWork/BFW_Source/BFW_WindowManager/WM_Cursor \
    -IBungieFrameWork/BFW_Source/BFW_WindowManager/WM_Dialog \
    -IBungieFrameWork/BFW_Source/BFW_WindowManager/WM_PartSpecification \
    -IBungieFrameWork/BFW_Source/BFW_WindowManager/WM_Utilities \
    -IBungieFrameWork/BFW_Source/BFW_WindowManager/WM_Windows \
    -IBungieFrameWork/BFW_Source/BFW_WindowManager/WM_DrawContext \
    tests/test_text_encoding.c -Wl,-dead_strip -o "$TMP/test_text_encoding"
"$TMP/test_text_encoding"
