// same includes as sh.c

#include "kernel/types.h"
#include "user/user.h"
#include "kernel/fcntl.h"

#define MAXBUF 100
#define MAXARGS 10

void
redir(int mode, char *buf) {
	// remove leading spaces
	while(*buf == ' ') {
		buf++;
	}
	char newbuf[MAXBUF-1];
	int i;
	for (i = 0; *buf != ' ' && *buf != 0; i++) {
		if (i >= MAXBUF-1) {
			fprintf(2, "nsh: redir: ran out of newbuf\n");
			exit(1);
		}
		newbuf[i] = *buf;
		buf++;
	}
	newbuf[i] = 0;

	if (strcmp(newbuf, "") == 0) {
		fprintf(2, "nsh: redir: missing file for redirection ");
		if (mode) {
			fprintf(2, "(>)\n");
		} else {
			fprintf(2, "(<)\n");
		}
		exit(1);
	}

	int fd;
	close(mode);
	if (mode) {
		fd = open(newbuf, O_WRONLY|O_CREATE);
		if (fd != 1) {
			fprintf(2, "nsh: redir: '>' fd is %d\n", fd);
			fprintf(2, "            the string is %s\n", newbuf);
			exit(1);
		}
	} else {
		fd = open(newbuf, O_RDONLY);
		if (fd != 0) {
			fprintf(2, "nsh: redir: '<' fd is %d\n", fd);
			fprintf(2, "            the string is %s\n", newbuf);
			exit(1);
		}
	}
}

void
getargs(char *buf, char **argv) {
	int ignore = 0;
	int reading = 0;
	int argc = 0;

	for (; *buf != 0; buf++) {
		if (*buf == ' ' || *buf == '<' || *buf == '>') {
			if (*buf == '<' || *buf == '>') {
				ignore = 1;
			}
			if (reading) {
				*buf = 0;
				reading = 0;
			}
		} else if (!reading) {
			if (ignore) {
				ignore = 0;
			} else {
				if (argc >= MAXARGS) {
					fprintf(2, "sh: too many args\n");
					exit(1);
				}
				*argv = buf;
				argv++;
				argc++;
			}
			reading = 1;
		}
	}

	*argv = 0;
}

void
execute(char *argv[]) {
	// check argv[0] (the command)
	if (argv[0] == 0) {
		exit(0); // if nothing entered, do nothing and exit
	} else { // if not blank, execute it
		if (exec(argv[0], argv) < 0) {
			fprintf(2, "failed to exec %s\n", argv[0]);
			exit(1);
		}
	}
}

void dopipe(char *rest);

void
parse(char *buf) {
	// remove leading spaces
	while (*buf == ' ') {
		buf++;
	}

	int k;
	// check for a pipe
	for (k = 0; buf[k] != 0; k++) {
		if (buf[k] == '|') {
			buf[k] = 0; // seperate into two strings
			dopipe(&buf[k+1]);
			break;
		}
	}

	// remove ending spaces
	for (k--; buf[k] == ' '; k--);
	buf[k+1] = 0;

	// check for redirection
	for (; k>=0; k--) {
		if (buf[k] == '<') {
			redir(0, &buf[k+1]); // redirect input
		} else if (buf[k] == '>') {
			redir(1, &buf[k+1]); // redirect output
		}
	}

	char *argv[MAXARGS]; // create array of args
	getargs(buf, argv);  // get the args
	execute(argv);       // execute the args
}

int
safe_fork() {
	int result = fork();

	if (result < 0) {
		fprintf(2, "forking error!\n");
		exit(1);
	}

	return result;
}

void
dopipe(char *rest) {
	int pipe_fd[2];
	pipe(pipe_fd);

	if (safe_fork() == 0) {
		close(1);        // close stdout
		dup(pipe_fd[1]); // make pipe_fd[1] the new stdout
		close(pipe_fd[0]);     // close old file descriptors
		close(pipe_fd[1]);
		// continue executing where you left off in parse
	} else {// execute a new parse for the rest
		if (safe_fork() == 0) {
			close(0);        // close stdin
			dup(pipe_fd[0]); // make pipe_fd[0] the new stdin
			close(pipe_fd[0]);     // close old file descriptors
			close(pipe_fd[1]);
			parse(rest);
		}

		close(pipe_fd[0]); // close pipe so that no one waits for it
		close(pipe_fd[1]);
		wait(0);
		wait(0);
		exit(0);
	}
}

int
getline(char buf[]) {
	int result;
	char c;
	int k = 0;

	result = read(0,&c,1);
	if (result == 0)
		return -1;
	while (result > 0 && c == ' ') {
		result = read(0,&c,1);
	}
	while (result > 0 && c != '\n') {
		if (k >= MAXBUF) {
			fprintf(2, "nsh: getline: buf out of space\n");
			exit(1);
		}
		buf[k] = c;
		k++;
		result = read(0,&c,1);
	}
	if (result < 0) {
		fprintf(2, "nsh: getline: read syscall error :o\n");
		exit(1);
	}
	buf[k] = 0;
	return 0;
}

int
main(int argc, char *argv[])
{
	char buf[MAXBUF];

	fprintf(1, "@ ");
	while (getline(buf) >= 0) {
		if (buf[0] == 'c' && buf[1] == 'd' && buf[2] == ' ') {
			// Chdir must be called by parent, not child
			if(chdir(buf+3) < 0)
				fprintf(2, "cannot cd %s\n", buf+3);
		} else {
			if (safe_fork() == 0)
				parse(buf);
			wait(0);
		}
		fprintf(1, "@ ");
	}
	exit(0);
}


///////////////////////////
//garbage i want to keep //
///////////////////////////

/*
void
cd_command (char *argv[]) {
	if (strcmp(argv[0], "cd") != 0) {
		fprintf(2, "how did you get to cd: %s\n", argv[0]);
		exit(1);
	}

	char buf[MAXBUF-3];
	int k = 0;

	for(int i = 1; argv[i] != 0; i++) {
		for(int j = 0; argv[i][j] != 0; j++) {
			buf[k] = argv[i][j];
			k++;
		}
		buf[k] = ' ';
		k++;
	}
	buf[k-1] = 0;
	if(chdir(buf) < 0) {
		fprintf(2, "cannot cd %s\n", buf);
		exit(1);
	}
	fprintf(2, "did cd %s\n", buf);
	exit(0);
}
*/
