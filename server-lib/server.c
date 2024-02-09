#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <regex.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "server.h"

#define RETURN_IF_ERROR(code)                                                  \
  ({                                                                           \
    if (code < 0) {                                                            \
      log_error_code(code);                                                    \
      return code;                                                             \
    }                                                                          \
  })

#define RETURN_CODE_IF_ERROR(code, propagated_code)                            \
  ({                                                                           \
    if (code < 0) {                                                            \
      log_error_code(code);                                                    \
      return propagated_code;                                                  \
    }                                                                          \
  })

struct http_request {
  char *file_name;
  char *file_ext;
  int is_get_request;
};

void log_http_request(struct http_request *request) {
  if (INFO_LOGGING_ENABLED) {
    if (request->is_get_request) {
      printf("GET ");
    } else {
      printf("UNKOWN ");
    }
    printf("REQUEST RECEIVED: \"%s\"\n", request->file_name);
  }
}

struct url_path {
  char *path;
  char *response;
};

struct url_register {
  struct url_path *paths;
  size_t size;
};

int is_in_register(const struct url_register *url_register,
                   struct url_path **path, const char *input_path) {
  for (int i = 0; i < url_register->size; i++) {
    if (!strcmp(url_register->paths[i].path, input_path)) {
      *path = &url_register->paths[i];
      return 1;
    }
  }

  return 0;
}

void register_url(struct url_register *url_register,
                  const struct url_path *path) {
  struct url_path *list = (struct url_path *)malloc((url_register->size + 1) *
                                                    sizeof(struct url_path));
  memcpy(list, url_register->paths,
         sizeof(struct url_path) * url_register->size);
  memcpy(&list[url_register->size], path, sizeof(struct url_path));
  url_register->size++;
  free(url_register->paths);
  url_register->paths = list;
}

void destroy_register(struct url_register *url_register) {
  free(url_register->paths);
  free(url_register);
}

int server_fd;
struct url_register *url_register;

void log_error_code(int code) {
  if (ERROR_LOGGING_ENABLED && code < 0) {
    printf("LOG - ERROR CODE: ");
    switch (code) {
    case EADDRINUSE:
      printf("EADDRINUSE");
    default:
      printf("UNKOWN ERROR CODE (%d)", code);
    }
    printf("\n");
  };
}

int setup_server(int port_number) {
  struct sockaddr_in server_addr;

  url_register = malloc(sizeof(struct url_register));
  url_register->size = 0;
  url_register->paths = NULL;

  // create server socket
  server_fd = socket(AF_INET, SOCK_STREAM, 0);
  RETURN_CODE_IF_ERROR(server_fd, ERR_CODE_SOCKET_FAILED);
  RETURN_CODE_IF_ERROR(
      setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)),
      ERR_CODE_SET_SOCKET_PROP_FAILED);

  // config socket
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port_number);

  RETURN_CODE_IF_ERROR(
      bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)),
      ERR_CODE_BIND_FAILED);

  RETURN_CODE_IF_ERROR(listen(server_fd, SOCKET_MAX_QUEUE),
                       ERR_CODE_LISTEN_FAILED);

  return 0;
}

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

void *handle_client(void *arg) {
  int client_fd = *((int *)arg);
  char *buffer = (char *)malloc(MAX_HTTP_RESPONSE_SIZE * sizeof(char));

  // receive request data from client and store into buffer
  ssize_t bytes_received = recv(client_fd, buffer, MAX_HTTP_RESPONSE_SIZE, 0);
  if (bytes_received > 0) {
    struct http_request request = process_http_request(buffer);
    log_http_request(&request);

    char *response = (char *)malloc(MAX_HTTP_RESPONSE_SIZE * 2 * sizeof(char));
    size_t response_len;

    struct url_path *target_path;

    if (request.is_get_request &&
        is_in_register(url_register, &target_path, request.file_name)) {
      // build HTTP response
      build_http_response(request.file_name, request.file_ext, response,
                          &response_len, target_path->response);
    } else {
      build_404_response(response, &response_len);
    }

    // send HTTP response to client
    send(client_fd, response, response_len, 0);

    free(response);
    free(request.file_name);
    free(request.file_ext);
  }

  close(client_fd);
  free(arg);
  free(buffer);
  return NULL;
}

int run_server(int *request_count) {
  while (1) {
    // client info
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    int *client_fd = malloc(sizeof(int));

    // accept client connection
    if ((*client_fd = accept(server_fd, (struct sockaddr *)&client_addr,
                             &client_addr_len)) < 0) {
      perror("accept failed");
      continue;
    }

    // create a new thread to handle client request
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, handle_client, (void *)client_fd);
    pthread_detach(thread_id);
    (*request_count)++;
  }

  return 0;
}

int clean_up_server() {
  RETURN_CODE_IF_ERROR(close(server_fd), ERR_CODE_CLOSE_FAILED);
  destroy_register(url_register);

  return 0;
}

void sigint_callback() {
  log_error_code(clean_up_server());
  exit(0);
}

// Returns error code or the number of requests it successfully handled
int start_server() {
  int request_count = 0;

  signal(SIGINT, sigint_callback);

  // Run server
  RETURN_IF_ERROR(run_server(&request_count));

  // Close connection
  RETURN_IF_ERROR(clean_up_server());

  return request_count;
}

int python_register_url(char *path, char *response) {
  if (INFO_LOGGING_ENABLED) {
    printf("Registered path '%s' with response: %s\n", path, response);
  }

  // FIX Currently the allocations here are not freed properly, so mem leak
  struct url_path *url = (struct url_path *)malloc(sizeof(struct url_path));
  url->path = malloc(sizeof(char) * strlen(path));
  strcpy(url->path, path);
  url->response = malloc(sizeof(char) * strlen(response));
  strcpy(url->response, response);

  register_url(url_register, url);

  return 0;
}
