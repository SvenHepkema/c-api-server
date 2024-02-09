#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../server-lib/server-interface.h"

int lorem_ipsum(struct http_request *request, struct json_response *response) {
  char *message = "{\"message\": \"Lorem Ipsum\"}";

  response->response = (char *)malloc(sizeof(char) * strlen(message));
  strcpy(response->response, message);

  return 0;
}

int main() {
  setup_server(8080);

  register_path_callback("loremipsum", lorem_ipsum);
  start_server();
}
