#ifndef __TGALIB__
#define __TGALIB__

#include "qlumpy.h"

#define TGAHEADER_SIZE 18
#define QPALSIZE 768

typedef union {
    int rgba:32; 
    struct {
        byte b:8;
        byte g:8;
        byte r:8;
        byte a:8;
    } u;
} pixel_t;

extern byte cust_pal[QPALSIZE];
extern pixel_t pal_fallb[256];

// Replaces LoadLBM. Outputs an 8-bit indexed buffer and a 768-byte RGB palette.
void LoadTGA (char *filename, byte **picture, byte **palette, int *width, int *height);
void ReportPaletteQuality(char *filename);

#endif // __TGALIB__