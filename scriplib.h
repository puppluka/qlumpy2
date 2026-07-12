#ifndef __SCRIPLIB__
#define __SCRIPLIB__

#include "qlumpy.h"

#define MAXTOKEN 128
#define MAX_INCL_STACK 8

extern char token[MAXTOKEN];
extern char *scriptbuffer, *script_p, *scriptend_p;
extern char current_script_name[1024];
extern int grabbed;
extern int scriptline;
extern qboolean endofscript;

void LoadScriptFile (char *filename);
qboolean GetToken (qboolean crossline);
void UnGetToken (void);
qboolean TokenAvailable (void);

void PushScript(char *filename);
qboolean PopScript(void);

#endif // __SCRIPLIB__