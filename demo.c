#include "dmon.h"

int main(int argc, char *argv[])
{
  if (argc<2)  dmon_daemon();
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
