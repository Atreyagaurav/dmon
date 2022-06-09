#define DBUS_API_SUBJECT_TO_CHANGE
#include <dbus/dbus.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* Function to report to daemon. */
enum process_result{ INPROGRESS, CANCELLED, COMPLETED, FAILED};

struct dmon_process {
  int PID;
  int dmon_id;
  char *group;
  double progress;
  enum process_result result;
};

void print_dmon_prc(struct dmon_process* dp);
void sendsignal(struct dmon_process* proc);
void dmon_daemon();
