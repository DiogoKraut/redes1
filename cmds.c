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

void lls(void) {
	FILE *fp;
	char out[255]; 		// linux filename lenght limit
	char command[255] = "/bin/ls ./";

	/* Open ls process for reading */
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

int ls(int socket) {
	struct dirent **namelist;
	int size, i;
	tMessage *m = malloc(sizeof(tMessage));
	tMessage *mR = malloc(sizeof(tMessage));
	m->init = 0x7E;
    m->dest_addr = 0x1;
    m->src_addr  = 0x2;
    m->seq = 0;
    m->type = LS_DATA;
    m->size = 0;
    m->data[0] = '\0';

	size = scandir("./", &namelist, NULL, alphasort);
	if(size == -1) {
		printf("%s\n", strerror(errno));
		return -1;
	}

	for(i = 0; i < size; i++) {
		m->seq = i;
		if(strlen(namelist[i]->d_name) >= 15)
			m->size = 15;
		else
			m->size = strlen(namelist[i]->d_name);

		memcpy(m->data, namelist[i]->d_name, m->size);
		m->data[m->size] = '\0';
		m->parity = parity(m);
		if(!sendPacket(socket, m, mR, ACK))
			return -1;
	}
	m->size = 0;
	m->data[0] = '\0';
	m->type = EOTX;
	sendPacket(socket, m, mR, ACK);

	for(i = 0; i < size; i++) {
		free(namelist[i]);
	}
	free(namelist);
	free(m);
	free(mR);

	return 0;
}

