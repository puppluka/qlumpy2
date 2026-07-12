// cmdlib.h

#ifndef __CMDLIB__
#define __CMDLIB__

#ifndef __BYTEBOOL__
#define __BYTEBOOL__
#include <stdbool.h>
typedef bool qboolean;
typedef unsigned char byte;
#endif

#include "qlumpy.h"

// set these before calling CheckParm
extern int myargc;
extern char **myargv;

char *strupr (char *in);
char *strlower (char *in);
void Q_getwd (char *out);

int Q_filelength (FILE *f);
int	FileTime (char *path);

void	Q_mkdir (char *path);

extern	char		qdir[1024];
extern	char		gamedir[1024];
void SetQdirFromPath (char *path);
char *ExpandPath (char *path);
char *ExpandPathAndArchive (char *path);

void	Error (char *error, ...);

const char* GetExecutableName(const char* filepath);

int		CheckParm (char *check);

FILE	*SafeOpenWrite (char *filename);
FILE	*SafeOpenRead (char *filename);
void	SafeRead (FILE *f, void *buffer, int count);
void	SafeWrite (FILE *f, void *buffer, int count);

int		LoadFile (char *filename, void **bufferptr);
void	SaveFile (char *filename, void *buffer, int count);

void 	DefaultExtension (char *path, char *extension);
void 	DefaultPath (char *path, char *basepath);
void 	StripFilename (char *path);
void 	StripExtension (char *path);

void 	ExtractFilePath (char *path, char *dest);
void 	ExtractFileBase (char *path, char *dest);
void	ExtractFileExtension (char *path, char *dest);

int 	ParseNum (char *str);

int16_t	BigShort	(int16_t l);
int16_t	LittleShort	(int16_t l);
int32_t	BigLong		(int32_t l);
int32_t	LittleLong	(int32_t l);

float	BigFloat	(float l);
float	LittleFloat	(float l);

char *COM_Parse (char *data);

extern	char		com_token[1024];
extern	qboolean	com_eof;

void	CreatePath (char *path);
void Q_CopyFile (char *from, char *to);

extern	qboolean		archive;
extern	char			archivedir[1024];
#endif
