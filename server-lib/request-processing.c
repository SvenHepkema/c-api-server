#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <regex.h>
#include <unistd.h>
#include <netinet/in.h>

#include "constants.h"
#include "server.h"
#include "request-processing.h"

const char *get_file_extension(const char *file_name) {
  const char *dot = strrchr(file_name, '.');
  if (!dot || dot == file_name) {
    return "";
  }
  return dot + 1;
}

const char *get_mime_type(const char *file_ext) {
  if (strcasecmp(file_ext, "txt") == 0) {
    return "text/plain";
  } else if (strcasecmp(file_ext, "json") == 0) {
    return "text/json";
  } else {
    return "text/html";
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

void build_http_header(const char *file_name, const char *file_ext,
                       char *response, size_t *response_len) {
  // build HTTP header
  const char *mime_type = get_mime_type(file_ext);
  char *header = (char *)malloc(MAX_HTTP_RESPONSE_SIZE * sizeof(char));
  snprintf(header, MAX_HTTP_RESPONSE_SIZE,
           "HTTP/1.1 200 OK\r\n"
           "Content-Type: %s\r\n"
           "\r\n",
           mime_type);

  // copy header to response buffer
  *response_len = 0;
  memcpy(response, header, strlen(header));
  *response_len += strlen(header);

  free(header);
}

void build_http_body(const char *file_name, const char *file_ext,
                     char *response, size_t *response_len, char *message) {
  // copy body to response buffer
  memcpy(response + *response_len, message, strlen(message));
  *response_len += strlen(message);
}

void build_404_response(char *response, size_t *response_len) {
  snprintf(response, MAX_HTTP_RESPONSE_SIZE,
           "HTTP/1.1 404 Not Found\r\n"
           "Content-Type: text/plain\r\n"
           "\r\n"
           "404 Not Found");
  *response_len = strlen(response);
}

void build_http_response(const char *file_name, const char *file_ext,
                         char *response, size_t *response_len, char *message) {
  build_http_header(file_name, file_ext, response, response_len);
  build_http_body(file_name, file_ext, response, response_len, message);
}

struct http_request process_http_request(char *buffer) {
  // check if request is GET
  regex_t regex;
  regcomp(&regex, "^GET /([^ ]*) HTTP/1", REG_EXTENDED);
  regmatch_t matches[2];

  int is_get_request = regexec(&regex, buffer, 2, matches, 0) == 0;

  char *file_name, *file_ext;

  if (is_get_request) {
    // extract filename from request and decode URL
    buffer[matches[1].rm_eo] = '\0';
    const char *url_encoded_file_name = buffer + matches[1].rm_so;
    file_name = url_decode(url_encoded_file_name);

    // get file extension
    file_ext = (char *)malloc(sizeof(char) * 32);
    strcpy(file_ext, get_file_extension(file_name));
  }
  regfree(&regex);

  struct http_request request = {.file_name = file_name,
                                 .file_ext = file_ext,
                                 .is_get_request = is_get_request};

  return request;
}

struct request_handle {
	struct server_props* server;
	int client_fd;
};

void *handle_client(void *arg) {
  struct request_handle request_handle = *((struct request_handle *)arg);
  char *buffer = (char *)malloc(MAX_HTTP_RESPONSE_SIZE * sizeof(char));

  // receive request data from client and store into buffer
  ssize_t bytes_received = recv(request_handle.client_fd, buffer, MAX_HTTP_RESPONSE_SIZE, 0);
  if (bytes_received > 0) {
    struct http_request request = process_http_request(buffer);
    log_http_request(&request);

    char *response = (char *)malloc(MAX_HTTP_RESPONSE_SIZE * 2 * sizeof(char));
    size_t response_len;

    struct url_path *target_path;

    if (request.is_get_request &&
        is_in_register(request_handle.server->url_register, &target_path, request.file_name)) {
      // build HTTP response
      build_http_response(request.file_name, request.file_ext, response,
                          &response_len, target_path->response);
    } else {
      build_404_response(response, &response_len);
    }

    // send HTTP response to client
    send(request_handle.client_fd, response, response_len, 0);

    free(response);
    free(request.file_name);
    free(request.file_ext);
  }

  close(request_handle.client_fd);
  free(arg);
  free(buffer);
  return NULL;
}

int run_server(struct server_props* server, int *request_count) {
  while (1) {
    // client info
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    struct request_handle *request_handle = malloc(sizeof(struct request_handle));
		request_handle->server = server;

    // accept client connection
    if ((request_handle->client_fd = accept(server->server_fd, (struct sockaddr *)&client_addr,
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
