// Mouse-only gameplay sensitivity. Separate from persist.dat to preserve its retail layout.
#ifndef ONI_MOUSE_SENSITIVITY_H
#define ONI_MOUSE_SENSITIVITY_H
#include "BFW.h"

#define ONcMouseSensitivity_Min 0.05f
#define ONcMouseSensitivity_Max 2.0f
#define ONcMouseSensitivity_Default 1.0f
#define ONcMouseSensitivity_SliderMax 200
#define ONcMouseSensitivity_SliderDefault 160

// 80% of the logarithmic slider is devoted to slowing the original input down.
float ONrMouseSensitivity_FromSlider(int inPosition);
int ONrMouseSensitivity_ToSlider(float inSensitivity);
float ONrMouseSensitivity_Clamp(float inSensitivity);
float ONrMouseSensitivity_ReadFromPath(const char *inPath);
UUtBool ONrMouseSensitivity_WriteToPath(const char *inPath, float inSensitivity);

// Lazy-load once, apply changes immediately, save when Options closes.
float ONrMouseSensitivity_Get(void);
void ONrMouseSensitivity_Set(float inSensitivity);
UUtBool ONrMouseSensitivity_Save(void);
#endif
