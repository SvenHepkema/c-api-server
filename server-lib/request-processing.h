#include "server.h"
#include <stdlib.h>

#ifndef REQUEST_PROCESSING_H
#define REQUEST_PROCESSING_H

const char *get_file_extension(const char *file_name);
const char *get_mime_type(const char *file_ext);
char *url_decode(const char *src);

void build_http_header(struct http_request *request,
                       struct http_response *response);
void build_http_body(struct http_request *request,
                     struct http_response *response, struct url_path *path);
void build_404_response(struct http_response *response);
void build_http_response(struct http_request *request,
                         struct http_response *response, struct url_path *path);
void create_response(struct server_props *server, struct http_request *request,
                   struct http_response *response);

int process_http_request(char *buffer, struct http_request* request);

void *handle_client(void *arg);
int run_server(struct server_props *server, int *request_count);

#endif // REQUEST_PROCESSING_H
