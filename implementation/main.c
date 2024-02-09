#include <stdio.h>
#include <stdbool.h>

#include "../server-lib/server.h"


int main() {
	setup_server(8080);
	python_register_url("server", "<h1>Server</h1>");
	start_server();
}
