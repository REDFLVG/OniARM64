// Run the real SDL polling -> bindings -> action buffer path with stub devices.
#include "../BungieFrameWork/BFW_Source/BFW_LocalInput/BFW_LocalInput.c"
#include "../BungieFrameWork/BFW_Source/BFW_LocalInput/Platform_SDL/BFW_LI_Platform_SDL.c"
#include "ONi_BundlePath.h"
#include <math.h>

static Uint8 keys[SDL_NUM_SCANCODES];
static int mouse_x, mouse_y, checks;
const Uint8 *SDL_GetKeyboardState(int *count) { *count = SDL_NUM_SCANCODES; return keys; }
Uint32 SDL_GetRelativeMouseState(int *x, int *y) { *x = mouse_x; *y = mouse_y; return 0; }
SDL_Keycode SDL_GetKeyFromScancode(SDL_Scancode code) { (void)code; return SDLK_UNKNOWN; }
const char *SDL_GetScancodeName(SDL_Scancode code) { (void)code; return "test"; }
void LIrGamepad_GetData(LItAction *action) { (void)action; }
void UUcArglist_Call UUrStartupMessage(const char *format, ...) { (void)format; }
UUtError ONiBundlePath_ResolveStateFile(const char *name, char *path, size_t size)
{ (void)name; (void)path; (void)size; return UUcError_Generic; }
#define CHECK(c) do { checks++; if (!(c)) { \
    fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #c); exit(1); } } while (0)
int main(void)
{
    setenv("ONI_INPUT_TRACE", "0", 1);
    setenv("ONI_KEY_LAYOUT", "0", 1);
    LItActionDescription descriptions[] = {
        {LIcIT_Axis_Delta, LIc_Aim_LR, "aim_lr"},
        {LIcIT_Axis_Delta, LIc_Aim_UD, "aim_ud"},
        {LIcIT_Button, LIc_Bit_Forward, "forward"},
        {LIcIT_Button, LIc_Bit_StepLeft, "left"},
        {LIcIT_Button, LIc_Bit_Backward, "backward"},
        {LIcIT_Button, LIc_Bit_StepRight, "right"}
    };
    const int codes[] = {LIcMouseCode_XAxis, LIcMouseCode_YAxis, 'w', 'a', 's', 'd'};
    for (unsigned i = 0; i < 6; i++) LIgBindingArray[i] = (LItBinding){codes[i], &descriptions[i]};
    LItAction action;
    LIgMode_Internal = LIcMode_Game;
    LIgMouse_Invert = UUcFalse;
    for (int slider = 0; slider <= 200; slider++) {
        float sensitivity = ONrMouseSensitivity_FromSlider(slider);
        ONrMouseSensitivity_Set(sensitivity);
        mouse_x = 1; mouse_y = -1;
        LIrPlatform_PollInputForAction(&action);
        CHECK(action.buttonBits == 0); // mouse never creates a movement action
        CHECK(fabsf(action.analogValues[LIc_Aim_LR] - 0.5f * sensitivity) < 0.00001f);
        CHECK(fabsf(action.analogValues[LIc_Aim_UD] - 0.5f * sensitivity) < 0.00001f);
        keys[SDL_SCANCODE_W] = 1;
        LIrPlatform_PollInputForAction(&action);
        CHECK(action.buttonBits == LIc_BitMask_Forward);
        keys[SDL_SCANCODE_W] = 0;
        LIrPlatform_PollInputForAction(&action);
        CHECK(action.buttonBits == 0); // release clears previous action
        LIgMode_Internal = LIcMode_Normal;
        keys[SDL_SCANCODE_A] = 1;
        keys[SDL_SCANCODE_A] = 0; // released while a menu has focus
        LIgMode_Internal = LIcMode_Game;
        LIrPlatform_PollInputForAction(&action);
        CHECK(action.buttonBits == 0); // no residual movement after pause
    }
    printf("mouse sensitivity polling: %d checks passed (mouse + keyboard release + pause)\n", checks);
    return 0;
}
