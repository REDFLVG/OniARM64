// Exercise the actual Options event handler and numeric label updates.
#include "../OniProj/OniGameSource/Oni_OutGameUI/Oni_OutGameUI.c"
#include "ONi_BundlePath.h"
#include <stdlib.h>

static int checks, position;
static char title[64];
static char slider_storage, value_storage, dialog_storage;
#define SLIDER ((WMtWindow *)&slider_storage)
#define VALUE ((WMtWindow *)&value_storage)
#define DIALOG ((WMtDialog *)&dialog_storage)
#define CHECK(c) do { checks++; if (!(c)) { \
    fprintf(stderr, "FAIL line %d: %s\n", __LINE__, #c); exit(1); } } while (0)
UUtError ONiBundlePath_ResolveStateFile(const char *name, char *path, size_t size)
{ (void)name; (void)path; (void)size; return UUcError_Generic; }
WMtWindow *WMrDialog_GetItemByID(WMtDialog *dialog, UUtUns16 id)
{
    CHECK(dialog == DIALOG);
    return id == ONcOptions_Sldr_MouseSensitivity ? SLIDER :
        id == ONcOptions_Txt_MouseValue ? VALUE : NULL;
}
UUtBool WMrWindow_SetTitle(WMtWindow *window, const char *text, UUtUns16 capacity)
{
    CHECK(window == VALUE && capacity <= sizeof(title));
    snprintf(title, sizeof(title), "%s", text);
    return UUcTrue;
}
UUtInt32 WMrSlider_GetPosition(WMtSlider *slider)
{ CHECK(slider == SLIDER); return position; }
void WMrSlider_SetPosition(WMtSlider *slider, UUtInt32 value)
{ CHECK(slider == SLIDER); position = value; }
int main(void)
{
    ONiOGU_Options_UpdateMouseLabel(DIALOG);
    CHECK(strcmp(title, "1.000x") == 0);
    for (int i = 0; i <= 200; i++) {
        char expected[64];
        position = i;
        CHECK(ONiOGU_Options_HandleMouseCommand(DIALOG, ONcOptions_Sldr_MouseSensitivity,
            SLcNotify_NewPosition, SLIDER));
        snprintf(expected, sizeof(expected), "%.3fx", (double)ONrMouseSensitivity_FromSlider(i));
        CHECK(strcmp(title, expected) == 0);
        CHECK(ONrMouseSensitivity_Get() == ONrMouseSensitivity_FromSlider(i));
    }
    CHECK(strcmp(title, "2.000x") == 0);
    CHECK(ONiOGU_Options_HandleMouseCommand(DIALOG, ONcOptions_Btn_ResetMouse, WMcNotify_Click, NULL));
    CHECK(position == ONcMouseSensitivity_SliderDefault);
    CHECK(strcmp(title, "1.000x") == 0 && ONrMouseSensitivity_Get() == 1.0f);
    position = 0;
    CHECK(ONiOGU_Options_HandleMouseCommand(DIALOG, ONcOptions_Sldr_MouseSensitivity, 99, SLIDER));
    CHECK(strcmp(title, "1.000x") == 0); // ignore unrelated notification
    CHECK(!ONiOGU_Options_HandleMouseCommand(DIALOG, 999, WMcNotify_Click, NULL));
    printf("mouse sensitivity UI: %d checks passed (live value + reset)\n", checks);
    return 0;
}
