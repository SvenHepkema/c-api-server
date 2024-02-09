#include <stdlib.h>

#ifndef SERVER_H
#define SERVER_H

struct http_request {
  char *file_name;
  char *file_ext;
  int is_get_request;
};

void log_http_request(struct http_request *request);

struct url_path {
  char *path;
  char *response;
};

struct url_register {
  struct url_path *paths;
  size_t size;
};

int is_in_register(const struct url_register *url_register,
                   struct url_path **path, const char *input_path);
void register_url_in_register(struct url_register *url_register,
                              const struct url_path *path);
void destroy_register(struct url_register *url_register);


struct server_props {
	int port_number;
  int server_fd;
  struct url_register *url_register;
};

int create_server(struct server_props* server, int port);

int destroy_server(struct server_props* server);

#endif // SERVER_H
