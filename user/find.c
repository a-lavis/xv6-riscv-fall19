#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"

char*
rmpath(char *path)
{
	char *p;

	// Find first character after last slash.
	for(p=path+strlen(path); p >= path && *p != '/'; p--)
		;
	p++;
	return p;
}


void
find(char *path, char *target)
{
	char buf[512], *p;
	int fd;
	struct dirent de;
	struct stat st;

	if((fd = open(path, 0)) < 0){
		fprintf(2, "find: cannot open %s\n", path);
		exit(1);
	}

	if(fstat(fd, &st) < 0){
		fprintf(2, "find: cannot stat %s\n", path);
		close(fd);
		exit(1);
	}

	if(strcmp(rmpath(path), target) == 0){
		printf("%s\n", path);
	}

	if(st.type == T_DIR){
		// for each element of the directory,
		// call find(element, target)
		// unless the element is "." or ".."
		
		strcpy(buf, path);
		p = buf+strlen(buf);
		*p++ = '/';
		while(read(fd, &de, sizeof(de)) == sizeof(de)) {
				if(de.inum == 0)
					continue;
				if (strcmp(de.name, ".") == 0) {
					;
				} else if (strcmp(de.name, "..") == 0) {
					;
				} else {
					memmove(p, de.name, DIRSIZ);
					p[DIRSIZ] = 0;
					//printf("descending... %s\n", buf);
					find(buf, target);
				}
		}
	}
	close(fd);
}

int
main(int argc, char *argv[])
{
  if(argc < 3){
    printf("find: missing operand\n");
    exit(1);
  }
  if(argc > 3){
    printf("find: too many operands\n");
    exit(1);
  }

  find(argv[1], argv[2]);

  exit(0);
}
