#include "Oni_MouseSensitivity.h"
#include "ONi_BundlePath.h"
#include <ctype.h>
#include <errno.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static float ONgMouseSensitivity = ONcMouseSensitivity_Default;
static UUtBool ONgMouseSensitivityLoaded = UUcFalse;
static UUtBool ONgMouseSensitivityDirty = UUcFalse;

float ONrMouseSensitivity_Clamp(float value)
{
    if (!isfinite(value)) return ONcMouseSensitivity_Default;
    if (value < ONcMouseSensitivity_Min) return ONcMouseSensitivity_Min;
    if (value > ONcMouseSensitivity_Max) return ONcMouseSensitivity_Max;
    return value;
}

float ONrMouseSensitivity_FromSlider(int position)
{
    if (position <= 0) return ONcMouseSensitivity_Min;
    if (position >= ONcMouseSensitivity_SliderMax) return ONcMouseSensitivity_Max;
    if (position <= ONcMouseSensitivity_SliderDefault)
        return ONcMouseSensitivity_Min * powf(20.0f, position / 160.0f);
    return powf(2.0f, (position - 160) / 40.0f);
}

int ONrMouseSensitivity_ToSlider(float value)
{
    value = ONrMouseSensitivity_Clamp(value);
    return (int)lroundf(value <= 1.0f
        ? 160.0f * logf(value / ONcMouseSensitivity_Min) / logf(20.0f)
        : 160.0f + 40.0f * logf(value) / logf(2.0f));
}

float ONrMouseSensitivity_ReadFromPath(const char *path)
{
    char text[128], *end;
    size_t length;
    float value;
    FILE *file;
    if (path == NULL || (file = fopen(path, "r")) == NULL)
        return ONcMouseSensitivity_Default;
    length = fread(text, 1, sizeof(text) - 1, file);
    // Reject oversized files and embedded NULs, including valid-looking prefixes.
    if (ferror(file) || length == sizeof(text) - 1) {
        fclose(file);
        return ONcMouseSensitivity_Default;
    }
    fclose(file);
    text[length] = '\0';
    for (size_t i = 0; i < length; i++)
        if (text[i] == '\0') return ONcMouseSensitivity_Default;
    errno = 0;
    value = strtof(text, &end);
    if (end == text || errno != 0 || !isfinite(value)) return ONcMouseSensitivity_Default;
    while (isspace((unsigned char)*end)) end++;
    if (*end != '\0' || value < ONcMouseSensitivity_Min || value > ONcMouseSensitivity_Max)
        return ONcMouseSensitivity_Default;
    return value;
}

UUtBool ONrMouseSensitivity_WriteToPath(const char *path, float value)
{
    FILE *file;
    int wrote;
    if (path == NULL || (file = fopen(path, "w")) == NULL) return UUcFalse;
    // Nine significant digits round-trip every float slider value.
    wrote = fprintf(file, "%.9g\n", (double)ONrMouseSensitivity_Clamp(value));
    if (fclose(file) != 0) return UUcFalse;
    return wrote > 0;
}

float ONrMouseSensitivity_Get(void)
{
    if (!ONgMouseSensitivityLoaded) {
        char path[1024];
        if (ONiBundlePath_ResolveStateFile("mouse_sensitivity.txt", path, sizeof(path)) == UUcError_None)
            ONgMouseSensitivity = ONrMouseSensitivity_ReadFromPath(path);
        ONgMouseSensitivityLoaded = UUcTrue;
    }
    return ONgMouseSensitivity;
}

void ONrMouseSensitivity_Set(float value)
{
    float previous = ONrMouseSensitivity_Get();
    ONgMouseSensitivity = ONrMouseSensitivity_Clamp(value);
    if (ONgMouseSensitivity != previous) ONgMouseSensitivityDirty = UUcTrue;
}

UUtBool ONrMouseSensitivity_Save(void)
{
    char path[1024];
    if (!ONgMouseSensitivityDirty) return UUcTrue;
    if (ONiBundlePath_ResolveStateFile("mouse_sensitivity.txt", path, sizeof(path)) != UUcError_None ||
        !ONrMouseSensitivity_WriteToPath(path, ONgMouseSensitivity)) return UUcFalse;
    ONgMouseSensitivityDirty = UUcFalse;
    return UUcTrue;
}
