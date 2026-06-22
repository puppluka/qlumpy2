#ifndef __TGALIB__
#define __TGALIB__

#include "qlumpy.h"

// Replaces LoadLBM. Outputs an 8-bit indexed buffer and a 768-byte RGB palette.
void LoadTGA (char *filename, byte **picture, byte **palette, int *width, int *height);

#endif // __TGALIB__