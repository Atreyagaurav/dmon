#include "dmon.h"
#include "c_progress/c_progress.h"
#include <stdio.h>

DBusConnection* conn;


void print_dmon_prc(struct dmon_process* dp){
  printf("%s (%d): %.2f%%\n", dp->group, dp->dmon_id, dp->progress);
}

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


struct dmon_process* dmon_get_next(){
   DBusMessage* msg;
   DBusMessageIter args;
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
	 int i, type;
	 struct dmon_process* dp = malloc(sizeof(struct dmon_process));
	 
	 for (i=0; i<3; i++){
	   type = dbus_message_iter_get_arg_type(&args);
	   if (type == DBUS_TYPE_STRING){
	     dbus_message_iter_get_basic(&args, &dp->group);
	   }else if (type == DBUS_TYPE_UINT32){
	     dbus_message_iter_get_basic(&args, &dp->dmon_id);
	   }else if (type == DBUS_TYPE_DOUBLE){
	     dbus_message_iter_get_basic(&args, &dp->progress);
	   }
	   dbus_message_iter_next(&args);
	 }
	 // free the message
	 dbus_message_unref(msg);
	 return dp;
      }

      // free the message
      dbus_message_unref(msg);
   }
}

/* daemon */
void init_dmon_daemon(){
   DBusError err;
   int ret;
   
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

}

void dmon_close(){
   // close the connection
   dbus_connection_close(conn);
}


void dmon_daemon(int num_procs){
  struct dmon_process *dp;
  struct progress_bar *pb;
  char error[200];
  
  init_dmon_daemon();
  init_progress_bars(num_procs, 1, 100, 200, 1);
  while (1) {
    dp = dmon_get_next();
    if (dp->dmon_id < 0 || dp->dmon_id >= num_procs){
      sprintf(error, "Process not Found: %.2f%% (gp:%s; id:%d)",dp->progress, dp->group, dp->dmon_id);
      update_status(0, error);
      print_all_progress();
      continue;
    }
    pb = get_progress_bar(dp->dmon_id);
    if (pb->status == QUEUED){
      start_progress_bar(pb->index, dp->group); /* using group as label for now. */
    }
    update_progress_bar(dp->dmon_id, dp->progress);
    print_all_progress();
  }
  dmon_close();
}
