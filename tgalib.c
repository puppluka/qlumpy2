#include "tgalib.h"

byte cust_pal[QPALSIZE];
byte c_last; 
pixel_t last_px; 
qboolean palette_initialized = false;

double total_distortion = 0.0;
int pixel_count = 0;

/* Fallback palette from Quake, put into the Public Domain by John Carmack */
pixel_t pal_fallb[256] = {
	{0x000000},{0x0f0f0f},{0x1f1f1f},{0x2f2f2f},{0x3f3f3f},{0x4b4b4b},{0x5b5b5b},{0x6b6b6b},
	{0x7b7b7b},{0x8b8b8b},{0x9b9b9b},{0xababab},{0xbbbbbb},{0xcbcbcb},{0xdbdbdb},{0xebebeb},
	{0x0f0b07},{0x170f0b},{0x1f170b},{0x271b0f},{0x2f2313},{0x372b17},{0x3f2f17},{0x4b371b},
	{0x533b1b},{0x5b431f},{0x634b1f},{0x6b531f},{0x73571f},{0x7b5f23},{0x836723},{0x8f6f23},
	{0x0b0b0f},{0x13131b},{0x1b1b27},{0x272733},{0x2f2f3f},{0x37374b},{0x3f3f57},{0x474767},
	{0x4f4f73},{0x5b5b7f},{0x63638b},{0x6b6b97},{0x7373a3},{0x7b7baf},{0x8383bb},{0x8b8bcb},
	{0x000000},{0x070700},{0x0b0b00},{0x131300},{0x1b1b00},{0x232300},{0x2b2b07},{0x2f2f07},
	{0x373707},{0x3f3f07},{0x474707},{0x4b4b0b},{0x53530b},{0x5b5b0b},{0x63630b},{0x6b6b0f},
	{0x070000},{0x0f0000},{0x170000},{0x1f0000},{0x270000},{0x2f0000},{0x370000},{0x3f0000},
	{0x470000},{0x4f0000},{0x570000},{0x5f0000},{0x670000},{0x6f0000},{0x770000},{0x7f0000},
	{0x131300},{0x1b1b00},{0x232300},{0x2f2b00},{0x372f00},{0x433700},{0x4b3b07},{0x574307},
	{0x5f4707},{0x6b4b0b},{0x77530f},{0x835713},{0x8b5b13},{0x975f1b},{0xa3631f},{0xaf6723},
	{0x231307},{0x2f170b},{0x3b1f0f},{0x4b2313},{0x572b17},{0x632f1f},{0x733723},{0x7f3b2b},
	{0x8f4333},{0x9f4f33},{0xaf632f},{0xbf772f},{0xcf8f2b},{0xdfab27},{0xefcb1f},{0xfff31b},
	{0x0b0700},{0x1b1300},{0x2b230f},{0x372b13},{0x47331b},{0x533723},{0x633f2b},{0x6f4733},
	{0x7f533f},{0x8b5f47},{0x9b6b53},{0xa77b5f},{0xb7876b},{0xc3937b},{0xd3a38b},{0xe3b397},
	{0xab8ba3},{0x9f7f97},{0x937387},{0x8b677b},{0x7f5b6f},{0x775363},{0x6b4b57},{0x5f3f4b},
	{0x573743},{0x4b2f37},{0x43272f},{0x371f23},{0x2b171b},{0x231313},{0x170b0b},{0x0f0707},
	{0xbb739f},{0xaf6b8f},{0xa35f83},{0x975777},{0x8b4f6b},{0x7f4b5f},{0x734353},{0x6b3b4b},
	{0x5f333f},{0x532b37},{0x47232b},{0x3b1f23},{0x2f171b},{0x231313},{0x170b0b},{0x0f0707},
	{0xdbc3bb},{0xcbb3a7},{0xbfa39b},{0xaf978b},{0xa3877b},{0x977b6f},{0x876f5f},{0x7b6353},
	{0x6b5747},{0x5f4b3b},{0x533f33},{0x433327},{0x372b1f},{0x271f17},{0x1b130f},{0x0f0b07},
	{0x6f837b},{0x677b6f},{0x5f7367},{0x576b5f},{0x4f6357},{0x475b4f},{0x3f5347},{0x374b3f},
	{0x2f4337},{0x2b3b2f},{0x233327},{0x1f2b1f},{0x172317},{0x0f1b13},{0x0b130b},{0x070b07},
	{0xfff31b},{0xefdf17},{0xdbcb13},{0xcbb70f},{0xbba70f},{0xab970b},{0x9b8307},{0x8b7307},
	{0x7b6307},{0x6b5300},{0x5b4700},{0x4b3700},{0x3b2b00},{0x2b1f00},{0x1b0f00},{0x0b0700},
	{0x0000ff},{0x0b0bef},{0x1313df},{0x1b1bcf},{0x2323bf},{0x2b2baf},{0x2f2f9f},{0x2f2f8f},
	{0x2f2f7f},{0x2f2f6f},{0x2f2f5f},{0x2b2b4f},{0x23233f},{0x1b1b2f},{0x13131f},{0x0b0b0f},
	{0x2b0000},{0x3b0000},{0x4b0700},{0x5f0700},{0x6f0f00},{0x7f1707},{0x931f07},{0xa3270b},
	{0xb7330f},{0xc34b1b},{0xcf632b},{0xdb7f3b},{0xe3974f},{0xe7ab5f},{0xefbf77},{0xf7d38b},
	{0xa77b3b},{0xb79b37},{0xc7c337},{0xe7e357},{0x7fbfff},{0xabe7ff},{0xd7ffff},{0x670000},
	{0x8b0000},{0xb30000},{0xd70000},{0xff0000},{0xfff393},{0xfff7c7},{0xffffff},{0x9f5b53}
};

/* Load palette.lmp if it exists, otherwise use fallback */
static void InitPalette() {
    if (palette_initialized) return;
    
    FILE *fPAL = fopen("palette.lmp", "rb");
    if (fPAL) {
        printf("Loaded custom palette lump!\n");
        fread(cust_pal, 1, QPALSIZE, fPAL);
        fclose(fPAL);
    } else {
        printf("No palette.lmp given, using fallback palette\n");
        for (int p = 0; p < 256; p++) {
            cust_pal[p * 3 + 0] = pal_fallb[p].u.r;
            cust_pal[p * 3 + 1] = pal_fallb[p].u.g;
            cust_pal[p * 3 + 2] = pal_fallb[p].u.b;
        }
    }
    palette_initialized = true;
}

void ReportPaletteQuality(char *filename) {
	if (pixel_count == 0) return;
	double rmse = sqrt(total_distortion / pixel_count);
	printf("Palette Quality for '%s': %.2f.\n", filename, rmse);

	if (rmse > 15.0)
		printf("ReportPaletteQuality: High color loss detected!\nSource file may need contrast adjustments or color correction!");

	total_distortion = 0;
	pixel_count = 0;
}

/* Quickly translate 24-bit RGB value to our palette */
static byte pal24to8(byte r, byte g, byte b) {
    byte c_red, c_green, c_blue, c_best = 255;
    int dist, best = 255 + 255 + 255;

    if ((last_px.u.r == r) && (last_px.u.g == g) && (last_px.u.b == b)) {
        return c_last;
    }

    last_px.u.r = r; last_px.u.g = g; last_px.u.b = b;

    for (int l = 0; l < 256; l++) {
        if ((cust_pal[l * 3 + 0] == r) && (cust_pal[l * 3 + 1] == g) && (cust_pal[l * 3 + 2] == b)) {
            c_last = l;
            return l;
        }

        c_red = abs(cust_pal[l * 3 + 0] - r);
        c_green = abs(cust_pal[l * 3 + 1] - g);
        c_blue = abs(cust_pal[l * 3 + 2] - b);
        dist = (c_red + c_green + c_blue);

        if (dist < best) {
            best = dist;
            c_best = l;
        }

		total_distortion += (double)(best * best);
		pixel_count++;
    }

    c_last = c_best;
    return c_best;
}

/* =================
LoadTGA
=================
*/
void LoadTGA (char *filename, byte **picture, byte **palette, int *width, int *height)
{
    byte *tga_buffer;
    int length = LoadFile(filename, (void **)&tga_buffer);
    
    if (length < TGAHEADER_SIZE) {
        Error ("LoadTGA: File %s is too short", filename);
    }

    int img_w = (tga_buffer[12]) | (tga_buffer[13] << 8);
    int img_h = (tga_buffer[14]) | (tga_buffer[15] << 8);

	if ((img_w & 15) || (img_h & 15))    // Power of 2 sizing validation guard
		printf("[WARNING] -- LoadTGA: File '%s'; Dimensions read: '%dx%d'; not multiples of 16.\nWill likely cause alignment issues in-game.\n", filename, img_w, img_h);
	if ((img_w & (img_w - 1)) != 0 || (img_h & (img_h - 1)) != 0)    // power-of-two sizing check
		printf("[WARNING] -- LoadTGA: File '%s'; Texture is NOT sized to power of 2!\n", filename);

    int bpp = tga_buffer[16] / 8; 
    
    *width = img_w;
    *height = img_h;

    byte *pic_buffer = malloc(img_w * img_h);
    byte *pal_buffer = malloc(QPALSIZE);

    if (!pic_buffer || !pal_buffer) {
        Error ("LoadTGA: Memory allocation failed");
    }

    // TYPE 1: 8-Bit Indexed TGA (Extract native palette)
    if (tga_buffer[2] == 1) {
        if (tga_buffer[1] != 1 || tga_buffer[16] != 8) {
            Error ("LoadTGA: Invalid 8-bit TGA format in %s", filename);
        }
        
        int cmap_len = (tga_buffer[5]) | (tga_buffer[6] << 8);
        int cmap_bpp = tga_buffer[7] / 8;
        byte *cmap_data = tga_buffer + TGAHEADER_SIZE + tga_buffer[0];
        
        // Extract the BGR TGA palette to our RGB format
        for (int i = 0; i < cmap_len && i < 256; i++) {
            pal_buffer[i * 3 + 0] = cmap_data[i * cmap_bpp + 2]; // R
            pal_buffer[i * 3 + 1] = cmap_data[i * cmap_bpp + 1]; // G
            pal_buffer[i * 3 + 2] = cmap_data[i * cmap_bpp + 0]; // B
        }

        byte *tga_pixels = cmap_data + (cmap_len * cmap_bpp);

        // Copy image data natively (Quake expects top-down, TGA is usually bottom-up)
        int done = 0;
        for (int row = img_h - 1; row >= 0; row--) {
            for (int col = 0; col < img_w; col++) {
                pic_buffer[done++] = tga_pixels[(row * img_w) + col];
            }
        }
    }
    // TYPE 2: 24/32-Bit RGB TGA (Map to our working palette)
    else if (tga_buffer[2] == 2) {
        if (tga_buffer[16] != 24 && tga_buffer[16] != 32) {
            Error ("LoadTGA: %s is not 24/32-bit RGB", filename);
        }

        InitPalette();
        memcpy(pal_buffer, cust_pal, QPALSIZE);

        byte *tga_pixels = tga_buffer + TGAHEADER_SIZE + tga_buffer[0];

        int done = 0;
        for (int row = img_h - 1; row >= 0; row--) {
            for (int col = 0; col < img_w; col++) {
                int px_offset = (row * (img_w * bpp)) + (col * bpp);
                pic_buffer[done++] = pal24to8(
                    tga_pixels[px_offset + 2], // R (TGA is BGR)
                    tga_pixels[px_offset + 1], // G
                    tga_pixels[px_offset + 0]  // B
                );
            }
        }
    } 
    else {
        Error ("LoadTGA: %s must be uncompressed Type 1 or Type 2 Targa", filename);
    }

    free(tga_buffer);

    *picture = pic_buffer;
    *palette = pal_buffer;
}