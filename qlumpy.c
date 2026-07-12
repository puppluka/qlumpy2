#include "qlumpy.h"

#define VERSION "0.48"  // EXPERIMENTAL RESTORATION
#define MAXLUMP 0x50000 // biggest possible lump, approximately 327.68 kilobytes.

byte            *byteimage, *lbmpalette;
int              byteimagewidth, byteimageheight;

char            basepath[1024];
char            lumpname[16];
char			destfile[1024];
byte            *lumpbuffer, *lump_p;

qboolean		savesingle;
qboolean		outputcreated;
qboolean		newwadmarker;

void GrabRaw (void);
void GrabPalette (void);
void GrabPic (void);
void GrabMip (void);
void GrabColormap (void);
void GrabColormap2 (void);

typedef struct
{
	char    *name;
	void    (*function) (void);
} command_t;

command_t       commands[] =
{
	{"palette",GrabPalette},
	{"colormap",GrabColormap},
	{"qpic",GrabPic},
	{"miptex",GrabMip},
	{"raw",GrabRaw},

	{"colormap2",GrabColormap2},    // unused
	{NULL,NULL}                     // list terminator
};

/*
==============
LoadScreen
==============
*/
void LoadScreen (char *name)
{
	char	*expanded;
	expanded = ExpandPathAndArchive (name);

	printf ("grabbing from %s...\n",expanded);
	LoadTGA (expanded, &byteimage, &lbmpalette, &byteimagewidth, &byteimageheight);
	ReportPaletteQuality(name); // palette color loss analyzer
}


/*
================
CreateOutput
================
*/
void CreateOutput (void)
{
	outputcreated = true;
	NewWad (destfile, false); // create the output WAD2 file
}

/*===============
WriteLump
===============*/
void WriteLump (int type, int compression)
{
	int size;

	if (!outputcreated) CreateOutput ();

    // dword align the size securely
	while ((intptr_t)lump_p & 3)
	{
		// ADD THIS GUARD: Check bounds before writing the padding byte
		if (lump_p - lumpbuffer >= MAXLUMP)
			Error ("Lump size exceeded %d during alignment padding!", MAXLUMP);

		*lump_p++ = 0;
	}

	size = lump_p - lumpbuffer;
	if (size > MAXLUMP) Error ("Lump size exceeded %d, memory corrupted!",MAXLUMP);

    // write the grabbed lump to the wadfile
	AddLump (lumpname,lumpbuffer,size,type, compression);
}

/*
===========
WriteFile
    Save as a seperate lmp file instead of as a wadfile
===========
*/
void WriteFile (void)
{
	char	filename[1024];
	char	*exp;

	sprintf (filename,"%s/%s.lmp", destfile, lumpname);
	exp = ExpandPath(filename);
	printf ("saved %s\n", exp);
	SaveFile (exp, lumpbuffer, lump_p-lumpbuffer);
}

/*
================
ParseScript
================
*/
void ParseScript (void)
{
	int			cmd;
	int			size;

	do
	{
		// get a command / lump name
		GetToken (true);
		if (endofscript) {
			if (PopScript()) continue; // return to parent script
			break;
		}

		if (!strcasecmp(token, "$INCL")) {
			GetToken(false);
			printf("Including script '%s'\n", token);
			PushScript(token);
			continue;
		}

		if (!strcasecmp (token,"$LOAD"))
		{
			GetToken (false);
			LoadScreen (token);
			continue;
		}

		if (!strcasecmp (token,"$DEST"))
		{
			GetToken (false);
			strcpy (destfile, ExpandPath(token));
			continue;
		}

		if (!strcasecmp (token,"$SINGLEDEST"))
		{
			GetToken (false);
			strcpy (destfile, token);
			savesingle = true;
			continue;
		}
		// new lump
		if (strlen(token) >= sizeof(lumpname) )
			Error ("\"%s\" is too long to be a lump name",token);
		memset (lumpname,0,sizeof(lumpname));
		strcpy (lumpname, token);
		for (size=0 ; size<sizeof(lumpname) ; size++)
			lumpname[size] = tolower(lumpname[size]);
		// get the grab command
		lump_p = lumpbuffer;
		GetToken (false);
		// call a routine to grab some data and put it in lumpbuffer
		// with lump_p pointing after the last byte to be saved
		for (cmd=0 ; commands[cmd].name ; cmd++)
			if ( !strcasecmp(token,commands[cmd].name) )
			{
				commands[cmd].function ();
				break;
			}

		if ( !commands[cmd].name )
			Error ("Unrecognized token '%s' at line %i",token,scriptline);

		grabbed++;

		if (savesingle)
			WriteFile ();
		else
		{
			int lump_type = TYP_LUMPY + cmd;
			if (newwadmarker) {
				if (!strcasecmp(commands[cmd].name, "raw"))
					lump_type = 67; //0x43
				else if (!strcasecmp(commands[cmd].name, "miptex"))
					lump_type = 68; //0x44
			}
			WriteLump (lump_type, 0);
		}

	} while (script_p < scriptend_p);
}

/*
=================
ProcessLumpyScript
    Loads a script file, then grabs everything from it
=================
*/
void ProcessLumpyScript (char *basename)
{
	char            script[256];

	printf ("qlumpy script: %s\n",basename);
	// create default destination directory
	strcpy (destfile, ExpandPath(basename));
	StripExtension (destfile);
	strcat (destfile,".wad"); // unless the script overrides, save in cwd
	savesingle = false; // save in a wadfile by default
	grabbed = 0;
	outputcreated = false;
	strcpy (script, basename); // read in the script file
	DefaultExtension (script, ".ls");
	LoadScriptFile (script);

	strcpy (basepath, basename);
	ParseScript ();				// execute load/grab commands

	if (!savesingle)
	{
		if (outputcreated) {
			WriteWad ();			// write out the wad directory
			printf ("%i lumps grabbed in a wad file\n",grabbed);
		} else
			printf ("No lumps grabbed. WADfile not created.\n");
	}
}

/*
==============================
main
==============================
*/
int main (int argc, char **argv)
{
	int i;
	const char *exe_name = GetExecutableName(argv[0]);

	printf ("QLumpy "VERSION"\nby John Carmack and Pup Luka\n\nCopyright (c) 1994 id Software\n");

	if (argc == 1) {
		printf ("Usage: %s [-archive directory] [-newwad] scriptfile [scriptfile ...]\n", exe_name);
		return 1;
	}

	lumpbuffer = malloc (MAXLUMP);

	archive = false;
	newwadmarker = false;
	i = 1;

	// Modernized Argument Parser
	while (i < argc && argv[i][0] == '-')
	{
		if (!strcmp(argv[i], "-archive"))
		{
			archive = true;
			strcpy (archivedir, argv[i+1]);
			printf ("Archiving source to: %s\n", archivedir);
			i += 2;
		}
		else if (!strcmp(argv[i], "-newwad"))
		{
			newwadmarker = true;
			printf ("Modern Quake 1 WAD2 mode enabled (MIPTEX = 0x44)\n");
			i++;
		}
		else
		{
			Error ("Unknown option: %s", argv[i]);
		}
	}

	// Process all remaining script files
	for ( ; i<argc ; i++)
	{
		SetQdirFromPath (argv[i]);
		ProcessLumpyScript (argv[i]);
	}
	return 0;
}
