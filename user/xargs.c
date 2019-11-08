#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/param.h"

char
getchar() {
	char buf[1];
	if (read(0,buf,1) == 0) {
		exit(0);
	}
	return buf[0];
}

int
main(int argc, char *argv[])
{
	if (argc < 2) {
		printf("xargs: not enough args\n");
		exit(1);
	}


	char *newargv[MAXARG];
	int i;

	for (i = 0; i < argc-1; i++) {
		newargv[i] = argv[i+1];
	}


	int k;
	char buf[100];
	char *p;
	char c;

	while (1) {
		p = buf;
		k = 0;
		while (1) {
			if (k > 98) {
				printf("xargs: buf ran out of space :(\n");
				exit(1);
			}
			if (i > MAXARG-2) {
				printf("xargs: newargv ran out of space :(\n");
				exit(1);
			}

			c = getchar();
			if (c == '\n') {
				buf[k] = 0;
				if (k > 0) {
					newargv[i] = p;
					newargv[i+1] = 0;
				} else {
					newargv[i] = 0;
				}
				break;
			} else if (c == ' ') {
				newargv[i] = p;
				i++;
				p = &buf[k+1];
				buf[k] = 0;
				k++;
			} else {
				buf[k] = c;
				k++;
			}
		}

		//printf("[pre-exec]");

		if (fork() == 0) {
			exec(newargv[0], newargv);
		} else {
			wait(0);
		}

		//printf("[post-exec]");

		i = argc-1;
	}

	exit(0);
}
