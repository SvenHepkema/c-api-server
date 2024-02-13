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
server_t *global_server;

void sigint_callback() {
  log_error_code(destroy_server(global_server));
  exit(0);
}

int setup_server(int port_number, int n_threads, int polling_delay) {
  global_server = (server_t *)malloc(sizeof(server_t));
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

int register_url(url_path_t *path) {
  if (INFO_LOGGING_ENABLED) {
    printf("Registered path '%s'\n", path->path);
  }

  register_url_in_register(global_server->url_register, path);

  return 0;
}

int register_path_callback(char *path,
                           int (*callback)(http_request_t *request,
                                           json_response_t *response)) {
  url_path_t *url = (url_path_t *)malloc(sizeof(url_path_t));
  url->path = malloc(sizeof(char) * strlen(path));
  strcpy(url->path, path);
  url->callback = callback;

  register_url(url);

  return 0;
}
