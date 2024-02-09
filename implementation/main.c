#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "../server-lib/server-interface.h"

int home(struct http_request* request, struct json_response* response) {
	char* message = "<h1>Json Response</h1>";
	response->response = (char*) malloc(sizeof(char) * strlen(message));
	strcpy(response->response, message);

	return 0;
}

int main() {
	setup_server(8080);

	char * path = "server";
	char * response = "<h1>Server</h1>";


  // FIX Currently the allocations here are not freed properly, so mem leak
  struct url_path *url = (struct url_path *)malloc(sizeof(struct url_path));
  url->path = malloc(sizeof(char) * strlen(path));
  strcpy(url->path, path);
  url->callback = home;

	register_url(url);
	start_server();
}
