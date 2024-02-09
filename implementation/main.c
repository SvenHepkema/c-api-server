#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../server-lib/server-interface.h"

int lorem_ipsum(struct http_request *request, struct json_response *response) {
  char *message = "a";
  response->response = (char *)malloc(sizeof(char) * 2);
	memcpy(response->response, message, 2);

  return 0;
}

int main() {
  setup_server(8080);

  register_path_callback("loremipsum", lorem_ipsum);
  start_server();
}
