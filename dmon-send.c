#include "dmon.h"
#include "c_progress/c_progress.h"

int main(int argc, char *argv[])
{
  if (argc < 2){
    printf("Send progress signal to dmon-receive. \n\nUsage:"
	   " %s <LABEL> <ID> <PROGRESS>\n"
	   "   <LABEL>    Label for the progress bar.\n"
	   "   <ID>       ID of the progress bar (from 0).\n"
	   "   <PROGRESS> Progress Value of the progress bar (max 100.0).\n", argv[0]);
    exit(0);
  }
  struct dmon_process dp;
  dp.PID = getpid();
  dp.group = argv[1];
  dp.dmon_id = atoi(argv[2]);
  dp.progress = atof(argv[3]);
  print_dmon_prc(&dp);
  sendsignal(&dp);
  return 0;
}
