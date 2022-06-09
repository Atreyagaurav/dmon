#include "dmon.h"
#include "c_progress/c_progress.h"

int main(int argc, char *argv[])
{
  if (argc < 2){
    printf("Usage:    %s <NUM_BARS>\n"
	   "     <NUM_BARS> Number of bars to monitor.\n\n", argv[0]);
    exit(0);
  }

  dmon_daemon(atoi(argv[1]));
  return 0;
}
