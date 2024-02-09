#include <stdlib.h>

#ifndef REQUEST_PROCESSING_H
#define REQUEST_PROCESSING_H

const char *get_file_extension(const char *file_name);
const char *get_mime_type(const char *file_ext);
char *url_decode(const char *src);
void build_http_header(const char *file_name, const char *file_ext,
                       char *response, size_t *response_len);
void build_http_body(const char *file_name, const char *file_ext,
                     char *response, size_t *response_len, char *message);
void build_404_response(char *response, size_t *response_len);

void build_http_response(const char *file_name, const char *file_ext,
                         char *response, size_t *response_len, char *message);

struct http_request process_http_request(char *buffer);
void *handle_client(void *arg);
int run_server(struct server_props* server, int *request_count);

#endif // REQUEST_PROCESSING_H
