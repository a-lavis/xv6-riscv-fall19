#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(int argc, char *argv[])
{

  if(argc > 1){
    printf("pingpong: too many operands\n");
    exit(1);
  }

  int parent_fd[2];
  pipe(parent_fd);

  int child_fd[2];
  pipe(child_fd);

  char holder[1];

  write(parent_fd[1], "a", 1);

  if(fork() == 0) {
    read(parent_fd[0], holder, 1);
    if(holder[0] == 'a'){
      printf("%d: recieved ping\n", getpid());
    } else {
      printf("pingpong: did not recieve ping\n");
      exit(1);
    }
    write(child_fd[1], "b", 1);
    exit(0);
  } else {
    wait(0);
    read(child_fd[0], holder, 1);
    if(holder[0] == 'b'){
      printf("%d: recieved pong\n", getpid());
    } else {
      printf("pingpong: did not recieve pong\n");
      exit(1);
    }
  }

  exit(0);
}
