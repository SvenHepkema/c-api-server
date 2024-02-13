#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../server-lib/server-interface.h"

int lorem_ipsum(http_request_t *request, json_response_t *response) {
  char *message = "a";
  response->response = (char *)malloc(sizeof(char) * 2);
	memcpy(response->response, message, 2);

  return 0;
}

int main() {
  setup_server(8080, 1, 1000);

  register_path_callback("loremipsum", lorem_ipsum);
  start_server();
}
