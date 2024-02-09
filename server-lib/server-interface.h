#include "server.h"

#ifndef SERVER_INTERFACE_H
#define SERVER_INTERFACE_H

int register_url(struct url_path* path);
int setup_server(int port_number);
int start_server();

#endif // SERVER_INTERFACE_H
