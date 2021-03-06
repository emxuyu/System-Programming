/**
* Utilities Unleashed Lab
* CS 241 - Spring 2018
*/
#include <time.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include "format.h"

int main(int argc, char *argv[]) {
  if (argc < 2) {
    print_time_usage();
  }
  int status;
  struct timespec temp1, temp2;
  clockid_t clk_id = CLOCK_MONOTONIC;
  pid_t child = fork();
  if (child == -1) {
    print_fork_failed();
  }
  if (child > 0) {
    clock_gettime(clk_id, &temp1);
    pid_t pid = waitpid(child, &status, 0);
    clock_gettime(clk_id, &temp2);
    double duration = temp2.tv_sec - temp1.tv_sec + (temp2.tv_nsec - temp1.tv_nsec) / 1E9;
    if (pid != -1 && WIFEXITED(status)) {
      display_results(argv + 1, duration);
      return 0;
    }
  }
  else {
    execvp(argv[1], argv+1);
    print_exec_failed();
  }
}
