#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "constants.h"
#include "request-processing.h"
#include "server-interface.h"
#include "server.h"
#include "utils.h"

// FIX Find better solution for this
struct server_props *global_server;

void sigint_callback() {
  log_error_code(destroy_server(global_server));
  exit(0);
}

int setup_server(int port_number, int n_threads, int polling_delay) {
  global_server = (struct server_props *)malloc(sizeof(struct server_props));
  return create_server(global_server, port_number, n_threads, polling_delay);
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

int register_url(struct url_path *path) {
  if (INFO_LOGGING_ENABLED) {
    printf("Registered path '%s'\n", path->path);
  }

  register_url_in_register(global_server->url_register, path);

  return 0;
}

int register_path_callback(char *path,
                           int (*callback)(struct http_request *request,
                                            struct json_response *response)) {
  struct url_path *url = (struct url_path *)malloc(sizeof(struct url_path));
  url->path = malloc(sizeof(char) * strlen(path));
  strcpy(url->path, path);
  url->callback = callback;

	register_url(url);

	return 0;
}
