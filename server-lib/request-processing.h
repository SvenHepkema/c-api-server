#include "server.h"
#include <stdlib.h>

#ifndef REQUEST_PROCESSING_H
#define REQUEST_PROCESSING_H

const char *get_file_extension(const char *file_name);
const char *get_mime_type(const char *file_ext);
char *url_decode(const char *src);

void build_http_header(http_request_t *request, http_response_t *response);
void build_http_body(http_request_t *request, http_response_t *response,
                     url_path_t *path);
void build_404_response(http_response_t *response);
void build_http_response(http_request_t *request, http_response_t *response,
                         url_path_t *path);
void create_response(server_t *server, http_request_t *request,
                     http_response_t *response);

int process_http_request(char *buffer, http_request_t *request);

void handle_client(void *arg);
int run_server(server_t *server, int *request_count);

#endif // REQUEST_PROCESSING_H
