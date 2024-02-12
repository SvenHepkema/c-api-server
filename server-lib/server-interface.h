#include "server.h"

#ifndef SERVER_INTERFACE_H
#define SERVER_INTERFACE_H

int register_url(struct url_path *path);
int register_path_callback(char *path,
                           int (*callback)(struct http_request *request,
                                           struct json_response *response));
int setup_server(int port_number, int n_threads, int polling_delay);
int start_server();

#endif // SERVER_INTERFACE_H
