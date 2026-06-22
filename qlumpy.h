#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <stdint.h>
#include <stddef.h>
#include <time.h>
#include <strings.h>
//#include <sys/file.h>

#ifdef WIN32
#include <direct.h> //WIN32 getcwd
#else
#include <unistd.h> //POSIX getcwd
#endif

#include "cmdlib.h"
#include "scriplib.h"
#include "tgalib.h"
#include "wadlib.h"

#define SCRN(x,y) (*(byteimage+(y)*byteimagewidth+x))

extern  byte    *byteimage, *lbmpalette;
extern  int     byteimagewidth, byteimageheight;

extern  byte    *lump_p;
extern  byte	*lumpbuffer;
extern	char	lumpname[];
