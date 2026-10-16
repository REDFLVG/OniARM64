// Compile the real action buffer so the test covers the input integration,
// not just the slider's arithmetic. Unused engine code is dead-stripped.
#include "../BungieFrameWork/BFW_Source/BFW_LocalInput/BFW_LocalInput.c"
#include "Oni_MouseSensitivity.h"
#include "ONi_BundlePath.h"
#include <math.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

static char state_path[1024];
static UUtBool resolve_ok = UUcTrue;
static int checks;
UUtError ONiBundlePath_ResolveStateFile(const char *name, char *path, size_t size)
{
    if (!resolve_ok || strcmp(name, "mouse_sensitivity.txt")) return UUcError_Generic;
    snprintf(path, size, "%s", state_path);
    return UUcError_None;
}
#define CHECK(c) do { checks++; if (!(c)) { \
    fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #c); exit(1); } } while (0)
#define NEAR(a,b) (fabsf((a)-(b)) < 0.00001f)
static void contents(const char *text)
{
    FILE *f = fopen(state_path, "w");
    CHECK(f != NULL);
    CHECK(fputs(text, f) >= 0);
    CHECK(fclose(f) == 0);
}
static LItActionDescription descriptions[] = {
    {LIcIT_Axis_Delta, LIc_Aim_LR, "aim_lr"},
    {LIcIT_Axis_Delta, LIc_Aim_UD, "aim_ud"},
    {LIcIT_Axis_Delta, LIc_Look_LR, "look_lr"},
    {LIcIT_Button, LIc_Bit_Fire1, "fire1"}
};
static float axis(UUtUns32 code, float value, int target)
{
    LItAction action = {0};
    LItDeviceInput device = {code, value};
    LIrActionBuffer_Add(&action, &device);
    CHECK(device.analogValue == value); // do not mutate raw device input
    return action.analogValues[target];
}
int main(int argc, char **argv)
{
    char directory[] = "/tmp/oni-mouse-test-XXXXXX";
    if (argc == 3 && !strcmp(argv[1], "--reload")) {
        snprintf(state_path, sizeof(state_path), "%s", argv[2]);
        CHECK(NEAR(ONrMouseSensitivity_Get(), 0.25f));
        return 0;
    }
    CHECK(mkdtemp(directory) != NULL);
    snprintf(state_path, sizeof(state_path), "%s/mouse_sensitivity.txt", directory);
    CHECK(ONrMouseSensitivity_Get() == 1.0f);
    CHECK(ONrMouseSensitivity_Save()); // no changes: do not create a new preference
    CHECK(access(state_path, F_OK) != 0);
    CHECK(ONrMouseSensitivity_ReadFromPath(NULL) == 1.0f);
    CHECK(!ONrMouseSensitivity_WriteToPath(NULL, 1.0f));
    const char *bad[] = {"", "garbage", "nan", "inf", "-inf", "0", "-1", "2.01", "0.049", "0.25junk", "0.25\n2", "1e-99"};
    for (unsigned i = 0; i < sizeof(bad)/sizeof(bad[0]); i++) {
        contents(bad[i]);
        CHECK(ONrMouseSensitivity_ReadFromPath(state_path) == 1.0f);
    }
    contents(" \t0.25\r\n");
    CHECK(ONrMouseSensitivity_ReadFromPath(state_path) == 0.25f);
    contents("0.25                                                                                                                            junk");
    CHECK(ONrMouseSensitivity_ReadFromPath(state_path) == 1.0f);
    FILE *f = fopen(state_path, "w");
    CHECK(f != NULL);
    CHECK(fwrite("0.25\0junk", 1, 9, f) == 9);
    CHECK(fclose(f) == 0);
    CHECK(ONrMouseSensitivity_ReadFromPath(state_path) == 1.0f);
    CHECK(ONrMouseSensitivity_Clamp(NAN) == 1.0f);
    CHECK(ONrMouseSensitivity_Clamp(INFINITY) == 1.0f);
    CHECK(ONrMouseSensitivity_Clamp(-3) == 0.05f);
    CHECK(ONrMouseSensitivity_Clamp(10) == 2.0f);
    CHECK(ONrMouseSensitivity_FromSlider(-100) == 0.05f);
    CHECK(ONrMouseSensitivity_FromSlider(9999) == 2.0f);
    CHECK(ONrMouseSensitivity_FromSlider(160) == 1.0f);
    float previous = 0;
    for (int i = 0; i <= ONcMouseSensitivity_SliderMax; i++) {
        float value = ONrMouseSensitivity_FromSlider(i);
        CHECK(value >= 0.05f && value <= 2.0f && value > previous);
        CHECK(ONrMouseSensitivity_ToSlider(value) == i);
        CHECK(ONrMouseSensitivity_WriteToPath(state_path, value));
        CHECK(ONrMouseSensitivity_ReadFromPath(state_path) == value);
        previous = value;
    }
    LIgBindingArray[0] = (LItBinding){LIcMouseCode_XAxis, &descriptions[0]};
    LIgBindingArray[1] = (LItBinding){LIcMouseCode_YAxis, &descriptions[1]};
    LIgBindingArray[2] = (LItBinding){LIcMouseCode_ZAxis, &descriptions[2]};
    LIgBindingArray[3] = (LItBinding){LIcKeyCode_LeftArrow, &descriptions[0]};
    LIgBindingArray[4] = (LItBinding){LIcMouseCode_Button1, &descriptions[3]};
    LIgBindingArray[5] = (LItBinding){LIcGamepadCode_RSX, &descriptions[0]};
    LIgBindingArray[6] = (LItBinding){LIcGamepadCode_RSY, &descriptions[1]};
    ONrMouseSensitivity_Set(0.05f);
    CHECK(NEAR(axis(LIcMouseCode_XAxis, 0.5f, LIc_Aim_LR), 0.025f));
    CHECK(NEAR(axis(LIcMouseCode_YAxis, -0.5f, LIc_Aim_UD), -0.025f));
    CHECK(axis(LIcMouseCode_ZAxis, 2, LIc_Look_LR) == 2); // wheel unchanged
    CHECK(axis(LIcKeyCode_LeftArrow, 1, LIc_Aim_LR) == 1); // keys unchanged
    CHECK(axis(LIcGamepadCode_RSX, 8, LIc_Aim_LR) == 8);
    CHECK(axis(LIcGamepadCode_RSY, -8, LIc_Aim_UD) == -8);
    LItAction action = {0};
    LItDeviceInput mouse = {LIcMouseCode_XAxis, 0.5f};
    for (int i = 0; i < 1000; i++) LIrActionBuffer_Add(&action, &mouse);
    CHECK(fabsf(action.analogValues[LIc_Aim_LR] - 25.0f) < 0.001f);
    mouse.input = LIcMouseCode_Button1; mouse.analogValue = 1;
    LIrActionBuffer_Add(&action, &mouse);
    CHECK(action.buttonBits & LIc_BitMask_Fire1);
    ONrMouseSensitivity_Set(1);
    CHECK(axis(LIcMouseCode_XAxis, -123.5f, LIc_Aim_LR) == -123.5f);
    ONrMouseSensitivity_Set(2);
    CHECK(axis(LIcMouseCode_XAxis, 123.5f, LIc_Aim_LR) == 247);
    // The same factor applies before all signed/symmetric binding operations.
    descriptions[0].inputType = LIcIT_Axis_Symmetric_Neg;
    CHECK(axis(LIcMouseCode_XAxis, 0.5f, LIc_Aim_LR) == -1);
    descriptions[0].inputType = LIcIT_Axis_Symmetric_Pos;
    CHECK(axis(LIcMouseCode_XAxis, 0.5f, LIc_Aim_LR) == 1);
    descriptions[0].inputType = LIcIT_Axis_Asymmetric;
    CHECK(axis(LIcMouseCode_XAxis, 0.5f, LIc_Aim_LR) == 1);
    ONrMouseSensitivity_Set(0.25f);
    resolve_ok = UUcFalse;
    CHECK(!ONrMouseSensitivity_Save());
    CHECK(ONrMouseSensitivity_Get() == 0.25f); // failure keeps live value + dirty flag
    resolve_ok = UUcTrue;
    CHECK(ONrMouseSensitivity_Save());
    CHECK(ONrMouseSensitivity_ReadFromPath(state_path) == 0.25f);
    pid_t child = fork();
    CHECK(child >= 0);
    if (child == 0) { execl(argv[0], argv[0], "--reload", state_path, NULL); _exit(99); }
    int status;
    CHECK(waitpid(child, &status, 0) == child);
    CHECK(WIFEXITED(status) && WEXITSTATUS(status) == 0); // fresh-process persistence
    CHECK(!ONrMouseSensitivity_WriteToPath("/no-such-oni-directory/mouse_sensitivity.txt", 1));
    CHECK(unlink(state_path) == 0);
    CHECK(rmdir(directory) == 0);
    printf("mouse sensitivity: %d checks passed (including real action buffer and restart)\n", checks);
    return 0;
}
