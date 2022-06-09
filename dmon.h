#define DBUS_API_SUBJECT_TO_CHANGE
#include <dbus/dbus.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct dmon_process {
  int PID;
  int dmon_id;
  char *group;
  double progress;
};

void print_dmon_prc(struct dmon_process* dp);
void sendsignal(struct dmon_process* proc);
void dmon_daemon();
