#include "dmon.h"
#include "c_progress/c_progress.h"

int main(int argc, char *argv[])
{
  if (argc < 2){
    printf("Use:    %s <NUM_BARS>\n"
	   "     OR %s <LABEL> <ID> <PROGRESS>\n\n", argv[0], argv[0]);
    exit(0);
  }
  if (argc<3)  {
    dmon_daemon(atoi(argv[1]));
    exit(0);
  }
  else {
    struct dmon_process dp;
    dp.PID = 0;
    dp.group = argv[1];
    dp.dmon_id = atoi(argv[2]);
    dp.progress = atof(argv[3]);
    print_dmon_prc(&dp);
    sendsignal(&dp);
  }
  return 0;
}
