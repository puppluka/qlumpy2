#include "tgalib.h"

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
		printf("[WARNING] -- LoadTGA: Texture is NOT sized to power of 2!", filename);

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