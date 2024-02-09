#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

#include "constants.h"
#include "utils.h"
#include "server.h"
#include "server-interface.h"
#include "request-processing.h"

// FIX Find better solution for this
struct server_props* global_server;

void sigint_callback() {
  log_error_code(destroy_server(global_server));
  exit(0);
}

int setup_server(int port_number) {
	global_server = (struct server_props*) malloc(sizeof(struct server_props));
	return create_server(global_server, port_number);
}

// Returns error code or the number of requests it successfully handled
int start_server() {
  int request_count = 0;

  signal(SIGINT, sigint_callback);

  // Run server
  RETURN_IF_ERROR(run_server(global_server, &request_count));

  // Close connection
  RETURN_IF_ERROR(destroy_server(global_server));

  return request_count;
}

int register_url(struct url_path* path) {
  if (INFO_LOGGING_ENABLED) {
    printf("Registered path '%s'\n", path->path);
  }

  register_url_in_register(global_server->url_register, path);

  return 0;
}
