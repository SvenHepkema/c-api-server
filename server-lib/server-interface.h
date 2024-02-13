#include "server.h"

#ifndef SERVER_INTERFACE_H
#define SERVER_INTERFACE_H

int register_url(url_path_t *path);
int register_path_callback(char *path,
                           int (*callback)(http_request_t *request,
                                           json_response_t *response));
int setup_server(int port_number, int n_threads, int polling_delay);
int start_server();

#endif // SERVER_INTERFACE_H
