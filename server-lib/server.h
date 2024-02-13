#include <stdlib.h>

#include "threadpool.h"

#ifndef SERVER_H
#define SERVER_H

typedef struct {
  char *file_name;
  char *file_ext;
  int is_get_request;
} http_request_t;

void destroy_request(http_request_t *request);

typedef struct {
  char *response;
  size_t response_len;
} http_response_t;

void destroy_response(http_response_t *response);

typedef struct {
  char *response;
} json_response_t;

void log_http_request(http_request_t *request);

typedef struct {
  char *path;
  int (*callback)(http_request_t *request, json_response_t *response);
} url_path_t;

typedef struct {
  url_path_t *paths;
  size_t size;
} url_register_t;

int is_in_register(const url_register_t *url_register, url_path_t **path,
                   const char *input_path);
void register_url_in_register(url_register_t *url_register,
                              const url_path_t *path);
void destroy_register(url_register_t *url_register);

typedef struct {
  int port_number;
  int server_fd;
  url_register_t *url_register;
  threadpool_t *threadpool;
} server_t;

int create_server(server_t *server, int port, int n_threads,
                  int polling_delay);

int destroy_server(server_t *server);

#endif // SERVER_H
