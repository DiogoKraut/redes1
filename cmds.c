#include "cmds.h"

void cd(char *cwd, char *path) {
	char temp[PATH_MAX];

	/* No path specified; return */
	if(path == NULL)
		return;

	/* If path isnt to Root, "." or "..": add '/ to cwd and concatenate desired path'  */
	if( path[0] != '/' && (strcmp(path, ".") != 0) && (strcmp(path, "..") != 0) )  {
		strcpy(temp, cwd);
		strcat(temp, "/");
		strcat(temp, path);
	} else { // else: path is already a complete argument to chdir() 
		strcpy(temp, path);
	}

	if(chdir(temp) < 0) {
		printf("%s\n", strerror(errno));
	}
}

void ls(char *cwd) {
	FILE *fp;
	char out[255]; 		// linux filename lenght limit
	char command[255] = "/bin/ls ";

	/* Open ls process for reading */
	strcat(command,  cwd);
	fp = popen(command, "r");
	if (fp == NULL) {
		printf("Failed to run command\n" );
		exit(1);
	}

	/* Read the output a line at a time - output it. */
	while (fgets(out, sizeof(out), fp) != NULL) {
		printf("%s", out);
	}

	pclose(fp);
}
