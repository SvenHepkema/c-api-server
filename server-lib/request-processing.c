#include <netinet/in.h>
#include <pthread.h>
#include <regex.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "constants.h"
#include "request-processing.h"
#include "server.h"

const char *get_file_extension(const char *file_name) {
  const char *dot = strrchr(file_name, '.');
  if (!dot || dot == file_name) {
    return "";
  }
  return dot + 1;
}

const char *get_mime_type(const char *file_ext) {
  if (strcasecmp(file_ext, "txt") == 0 || strcasecmp(file_ext, "json") == 0) {
    return "text/json";
  } else if (strcasecmp(file_ext, "html") == 0 ||
             strcasecmp(file_ext, "htm") == 0) {
    return "text/html";
  } else {
    return "text/plain";
  }
}

char *url_decode(const char *src) {
  size_t src_len = strlen(src);
  char *decoded = malloc(src_len + 1);
  size_t decoded_len = 0;

  // decode %2x to hex
  for (size_t i = 0; i < src_len; i++) {
    if (src[i] == '%' && i + 2 < src_len) {
      int hex_val;
      sscanf(src + i + 1, "%2x", &hex_val);
      decoded[decoded_len++] = hex_val;
      i += 2;
    } else {
      decoded[decoded_len++] = src[i];
    }
  }

  // add null terminator
  decoded[decoded_len] = '\0';
  return decoded;
}

void build_http_header(struct http_request *request,
                       struct http_response *response) {
  // build HTTP header
  const char *mime_type = get_mime_type(request->file_ext);
  char *header = (char *)malloc(MAX_HTTP_RESPONSE_SIZE * sizeof(char));
  snprintf(header, MAX_HTTP_RESPONSE_SIZE,
           "HTTP/1.1 200 OK\r\n"
           "Content-Type: %s\r\n"
           "\r\n",
           mime_type);

  // copy header to response buffer
  response->response_len = 0;
  memcpy(response->response, header, strlen(header));
  response->response_len += strlen(header);

  free(header);
}

void build_http_body(struct http_request *request,
                     struct http_response *response, struct url_path *path) {
  struct json_response *json =
      (struct json_response *)malloc(sizeof(struct json_response));
  path->callback(request, json);

  // copy body to response buffer
  memcpy(response->response + response->response_len, json->response,
         strlen(json->response));
  response->response_len += strlen(json->response);
}

void build_404_response(struct http_response *response) {
  snprintf(response->response, MAX_HTTP_RESPONSE_SIZE,
           "HTTP/1.1 404 Not Found\r\n"
           "Content-Type: text/plain\r\n"
           "\r\n"
           "404 Not Found");
  response->response_len = strlen(response->response);
}

void build_http_response(struct http_request *request,
                         struct http_response *response,
                         struct url_path *path) {
  build_http_header(request, response);
  build_http_body(request, response, path);
}

int process_http_request(char *buffer, struct http_request *request) {

  // check if request is GET
  regex_t regex;
  regcomp(&regex, "^GET /([^ ]*) HTTP/1", REG_EXTENDED);
  regmatch_t matches[2];

  request->is_get_request = regexec(&regex, buffer, 2, matches, 0) == 0;

  if (request->is_get_request) {
    // extract filename from request and decode URL
    buffer[matches[1].rm_eo] = '\0';
    const char *url_encoded_file_name = buffer + matches[1].rm_so;
    request->file_name = url_decode(url_encoded_file_name);

    // get file extension
    request->file_ext = (char *)malloc(sizeof(char) * 32);
    strcpy(request->file_ext, get_file_extension(request->file_name));
  }
  regfree(&regex);

  return 0;
}

struct handle_client_args {
  struct server_props *server;
  int client_fd;
};

void create_response(struct server_props *server, struct http_request *request,
                     struct http_response *response) {
  response->response =
      (char *)malloc(MAX_HTTP_RESPONSE_SIZE * 2 * sizeof(char));

  struct url_path *target_path;

  if (request->is_get_request &&
      is_in_register(server->url_register, &target_path, request->file_name)) {
    build_http_response(request, response, target_path);
  } else {
    build_404_response(response);
  }
}

void *handle_client(void *arg) {
  struct handle_client_args args = *((struct handle_client_args *)arg);
  char *buffer = (char *)malloc(MAX_HTTP_RESPONSE_SIZE * sizeof(char));

  // receive request data from client and store into buffer
  ssize_t bytes_received =
      recv(args.client_fd, buffer, MAX_HTTP_RESPONSE_SIZE, 0);
  if (bytes_received > 0) {
    struct http_request *request =
        (struct http_request *)malloc(sizeof(struct http_request));
    process_http_request(buffer, request);
    log_http_request(request);

    struct http_response *response =
        (struct http_response *)malloc(sizeof(struct http_response));
    create_response(args.server, request, response);

    // send HTTP response to client
    send(args.client_fd, response->response, response->response_len, 0);

    destroy_request(request);
    destroy_response(response);
  }

  close(args.client_fd);
  free(arg);
  free(buffer);
  return NULL;
}

int run_server(struct server_props *server, int *request_count) {
  while (1) {
    // client info
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    struct handle_client_args *request_handle =
        malloc(sizeof(struct handle_client_args));
    request_handle->server = server;

    // accept client connection
    if ((request_handle->client_fd =
             accept(server->server_fd, (struct sockaddr *)&client_addr,
                    &client_addr_len)) < 0) {
      perror("accept failed");
      continue;
    }

    // create a new thread to handle client request
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, handle_client, (void *)request_handle);
    pthread_detach(thread_id);
    (*request_count)++;
  }

  return 0;
}
