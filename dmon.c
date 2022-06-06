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

void print_dmon_prc(struct dmon_process* dp){
  printf("%s (%d): %.2f%%\n", dp->group, dp->dmon_id, dp->progress*100);
}

int dmon_register_new(char* group, char* label);

int dmon_report_progress(char* group, int proc_id, double progress);

int dmon_report_result(char* group, int proc_id, enum process_result result);

void sendsignal(struct dmon_process* proc)
{
   DBusMessage* msg;
   DBusMessageIter args;
   DBusConnection* conn;
   DBusError err;
   int ret;
   dbus_uint32_t serial = 0;

   // initialise the error value
   dbus_error_init(&err);

   // connect to the DBUS system bus, and check for errors
   conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
   if (dbus_error_is_set(&err)) { 
      fprintf(stderr, "Connection Error (%s)\n", err.message); 
      dbus_error_free(&err); 
   }
   if (conn == NULL) { 
      exit(1); 
   }

   // register our name on the bus, and check for errors
   ret = dbus_bus_request_name(conn, "dmon.source", DBUS_NAME_FLAG_REPLACE_EXISTING , &err);
   if (dbus_error_is_set(&err)) { 
      fprintf(stderr, "Name Error (%s)\n", err.message); 
      dbus_error_free(&err); 
   }
   if (ret != DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) { 
      exit(1);
   }

   // create a signal & check for errors 
   msg = dbus_message_new_signal("/dmon/Object", "dmon.Type", "Report");
   if (msg == NULL) { 
      fprintf(stderr, "Message Null\n"); 
      exit(1); 
   }

   // append arguments onto signal
   dbus_message_iter_init_append(msg, &args);
   if (!(dbus_message_iter_append_basic(&args, DBUS_TYPE_STRING, &proc->group) &&
	 dbus_message_iter_append_basic(&args, DBUS_TYPE_UINT32, &proc->dmon_id) &&
         dbus_message_iter_append_basic(&args, DBUS_TYPE_DOUBLE, &proc->progress))){
      fprintf(stderr, "Out Of Memory!\n"); 
      exit(1);
   }

   // send the message and flush the connection
   if (!dbus_connection_send(conn, msg, &serial)) {
      fprintf(stderr, "Out Of Memory!\n"); 
      exit(1);
   }
   dbus_connection_flush(conn);
   
   printf("Signal Sent\n");
   
   // free the message and close the connection
   dbus_message_unref(msg);
   dbus_connection_close(conn);
}

/* daemon */
void dmon_daemon(){
   DBusMessage* msg;
   DBusMessageIter args;
   DBusConnection* conn;
   DBusError err;
   int ret;
   char* sigvalue;
   
   dbus_error_init(&err);
   conn = dbus_bus_get(DBUS_BUS_SESSION, &err);
   if (dbus_error_is_set(&err)) { 
      fprintf(stderr, "Connection Error (%s)\n", err.message);
      dbus_error_free(&err); 
   }
   if (NULL == conn) { 
      exit(1);
   }
   
   // request our name on the bus and check for errors
   ret = dbus_bus_request_name(conn, "dmon.sink", DBUS_NAME_FLAG_REPLACE_EXISTING , &err);
   if (dbus_error_is_set(&err)) { 
      fprintf(stderr, "Name Error (%s)\n", err.message);
      dbus_error_free(&err); 
   }
   if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret) {
      exit(1);
   }

   // add a rule for which messages we want to see
   dbus_bus_add_match(conn, "type='signal',interface='dmon.Type'", &err); // see signals from the given interface
   dbus_connection_flush(conn);
   if (dbus_error_is_set(&err)) { 
      fprintf(stderr, "Match Error (%s)\n", err.message);
      exit(1); 
   }
   printf("Match rule sent\n");

   // loop listening for signals being emmitted
   while (true) {

      // non blocking read of the next available message
      dbus_connection_read_write(conn, 0);
      msg = dbus_connection_pop_message(conn);

      // loop again if we haven't read a message
      if (msg == NULL) { 
         usleep(1); 		/* TODO need to find optimum value for it to consume less CPU on idle.
				 And also not have too much delay on reporting.*/
         continue;
      }

      // check if the message is a signal from the correct interface and with the correct name
      if (dbus_message_is_signal(msg, "dmon.Type", "Report")) {
         
         // read the parameters
         if (!dbus_message_iter_init(msg, &args))
            fprintf(stderr, "Message Has No Parameters\n");
	 int i, type, dmon_id;
	 double prog;
	 
	 for (i=0; i<3; i++){
	   type = dbus_message_iter_get_arg_type(&args);
	   if (type == DBUS_TYPE_STRING){
	     dbus_message_iter_get_basic(&args, &sigvalue);
	   }else if (type == DBUS_TYPE_UINT32){
	     dbus_message_iter_get_basic(&args, &dmon_id);
	   }else if (type == DBUS_TYPE_DOUBLE){
	     dbus_message_iter_get_basic(&args, &prog);
	   }
	   dbus_message_iter_next(&args);
	 }
         printf("%s (%d): %.2f%%\n", sigvalue, dmon_id, prog*100);
      }

      // free the message
      dbus_message_unref(msg);
   }
   // close the connection
   dbus_connection_close(conn);
}

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
