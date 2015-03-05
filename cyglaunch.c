//
// Cygterm launcher
//
// Copyright (c)2007-2013 TeraTerm Project
//   http://ttssh2.sourceforge.jp/
//
// [How to compile]
// Cygwin:
//  # cc -mno-cygwin -mwindows -o cyglaunch cyglaunch.c
//

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>


#define Section "Tera Term"
char *FName = "TERATERM.INI";


//
// Connect to local cygwin
//
void OnCygwinConnection(char *CygwinDirectory, char *cmdline)
{
	char file[MAX_PATH], *filename;
	char c, *envptr, *envbuff;
	int envbufflen;
	char *exename = "cygterm.exe";
	char cmd[1024];
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	if (strlen(CygwinDirectory) > 0) {
		if (SearchPath(CygwinDirectory, "bin\\msys-2.0", ".dll", sizeof(file), file, &filename) > 0) {
			goto found_dll;
		}
	}

	if (SearchPath(NULL, "msys-2.0", ".dll", sizeof(file), file, &filename) > 0) {
		goto found_path;
	}

	for (c = 'C' ; c <= 'Z' ; c++) {
		char tmp[MAX_PATH];
		sprintf(tmp, "%c:\\msys\\bin;%c:\\msys64\\bin", c, c);
		if (SearchPath(tmp, "msys-2.0", ".dll", sizeof(file), file, &filename) > 0) {
			goto found_dll;
		}
	}

	MessageBox(NULL, "Can't find Cygwin directory.", "ERROR", MB_OK | MB_ICONWARNING);
	return;

found_dll:;
	envptr = getenv("PATH");
	file[strlen(file)-13] = '\0'; // delete "\\msys-2.0.dll"
	if (envptr != NULL) {
		envbufflen = strlen(file) + strlen(envptr) + 7; // "PATH="(5) + ";"(1) + NUL(1)
		if ((envbuff=malloc(envbufflen)) == NULL) {
			MessageBox(NULL, "Can't allocate memory.", "ERROR", MB_OK | MB_ICONWARNING);
			return;
		}
		snprintf(envbuff, envbufflen, "PATH=%s;%s", file, envptr);
	} else {
		envbufflen = strlen(file) + strlen(envptr) + 6; // "PATH="(5) + NUL(1)
		if ((envbuff=malloc(envbufflen)) == NULL) {
			MessageBox(NULL, "Can't allocate memory.", "ERROR", MB_OK | MB_ICONWARNING);
			return;
		}
		snprintf(envbuff, envbufflen, "PATH=%s", file);
	}
	putenv(envbuff);
	if (envbuff) {
		free(envbuff);
		envbuff = NULL;
	}

found_path:;
	memset(&si, 0, sizeof(si));
	GetStartupInfo(&si);
	memset(&pi, 0, sizeof(pi));

	strcpy(cmd, exename);
	strcat(cmd, " ");
	strncat(cmd, cmdline, sizeof(cmd)-strlen(cmd)-1);
//printf("%s", cmd);
//MessageBox(NULL, cmd, "", MB_OK);
	if (CreateProcess(
			NULL,
			cmd,
			NULL, NULL, FALSE, 0,
			NULL, NULL,
			&si, &pi) == 0) {
		MessageBox(NULL, "Can't execute Cygterm.", "ERROR", MB_OK | MB_ICONWARNING);
	}
}


int main(int argc, char** argv)
{
	char Temp[256], CygwinDir[256], Cmdline[256];
	char *bs;
	int i;
	BOOL d_opt=FALSE;

	if (GetModuleFileName(NULL, Temp, sizeof(Temp)) > 0 &&
	   (bs = strrchr(Temp, '\\')) != NULL) {
		*bs = 0;
		chdir(Temp);
		snprintf(bs, sizeof(Temp) + Temp - bs, "\\%s", FName);
	}
	else {
		snprintf(Temp, sizeof(Temp), ".\\", FName);
	}

	// Cygwin install path
 	GetPrivateProfileString(Section, "CygwinDirectory", "c:\\msys",
			  CygwinDir, sizeof(CygwinDir), Temp);

	//printf("%s %d\n", CygwinDir, GetLastError());

	Cmdline[0] = 0;
	for (i=1; i<argc; i++) {
		if (i != 1) {
			strncat(Cmdline, " ", sizeof(Cmdline)-strlen(Cmdline)-1);
		}
		if (d_opt && strncmp("\"\\\\", argv[i], 3) == 0) {
			argv[i][1] = '/';
			argv[i][2] = '/';
		}
		strncat(Cmdline, argv[i], sizeof(Cmdline)-strlen(Cmdline)-1);
		if (strcmp(argv[i], "-d") == 0) {
			d_opt = TRUE;
		}
		else {
			d_opt = FALSE;
		}
	}
	//printf("%s\n", Cmdline);

	OnCygwinConnection(CygwinDir, Cmdline);

	return 0;
}
