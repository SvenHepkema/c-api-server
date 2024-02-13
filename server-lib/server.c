#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "constants.h"
#include "server.h"
#include "utils.h"

void log_http_request(http_request_t *request) {
  if (INFO_LOGGING_ENABLED) {
    if (request->is_get_request) {
      printf("GET ");
    } else {
      printf("UNKOWN ");
    }
    printf("REQUEST RECEIVED: \"%s\"\n", request->file_name);
  }
}

void destroy_request(http_request_t *request) {
  free(request->file_name);
  free(request->file_ext);
  free(request);
}

void destroy_response(http_response_t *response) {
  free(response->response);
  free(response);
}

int is_in_register(const url_register_t *url_register, url_path_t **path,
                   const char *input_path) {
  for (int i = 0; i < url_register->size; i++) {
    if (!strcmp(url_register->paths[i].path, input_path)) {
      *path = &url_register->paths[i];
      return 1;
    }
  }

  return 0;
}

void register_url_in_register(url_register_t *url_register,
                              const url_path_t *path) {
  url_path_t *list =
      (url_path_t *)malloc((url_register->size + 1) * sizeof(url_path_t));
  memcpy(list, url_register->paths, sizeof(url_path_t) * url_register->size);
  memcpy(&list[url_register->size], path, sizeof(url_path_t));
  url_register->size++;
  free(url_register->paths);
  url_register->paths = list;
}

void destroy_register(url_register_t *url_register) {
  free(url_register->paths);
  free(url_register);
}

int create_server(server_t *server, int port, int n_threads,
                  int polling_delay) {
  struct sockaddr_in server_addr;

  server->port_number = port;

  server->url_register = malloc(sizeof(url_register_t));
  server->url_register->size = 0;
  server->url_register->paths = NULL;

  // Creating the threadpool
  server->threadpool = malloc(sizeof(threadpool_t));
  create_threadpool(n_threads, polling_delay, server->threadpool);

  // create server socket
  server->server_fd = socket(AF_INET, SOCK_STREAM, 0);
  RETURN_CODE_IF_ERROR(server->server_fd, ERR_CODE_SOCKET_FAILED);
  RETURN_CODE_IF_ERROR(setsockopt(server->server_fd, SOL_SOCKET, SO_REUSEADDR,
                                  &(int){1}, sizeof(int)),
                       ERR_CODE_SET_SOCKET_PROP_FAILED);

  // config socket
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(server->port_number);

  RETURN_CODE_IF_ERROR(bind(server->server_fd, (struct sockaddr *)&server_addr,
                            sizeof(server_addr)),
                       ERR_CODE_BIND_FAILED);

  RETURN_CODE_IF_ERROR(listen(server->server_fd, SOCKET_MAX_QUEUE),
                       ERR_CODE_LISTEN_FAILED);
  return 0;
}

int destroy_server(server_t *server) {
  RETURN_CODE_IF_ERROR(close(server->server_fd), ERR_CODE_CLOSE_FAILED);
  destroy_register(server->url_register);

  // Destroying the threadpool
  destroy_threadpool(server->threadpool);
  free(server->threadpool);

  return 0;
}
