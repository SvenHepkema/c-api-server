#ifndef SERVER_H
#define SERVER_H

// ================
// CONSTANTS
// ================

// General
static const bool ERROR_LOGGING_ENABLED = true;
static const bool INFO_LOGGING_ENABLED = true;
static const int SOCKET_MAX_QUEUE = 10;
static const int MAX_HTTP_RESPONSE_SIZE = 1024 * 1024;

// Error codes
static const int ERR_CODE_UNKNOWN = -1;
static const int ERR_CODE_SOCKET_FAILED = -2;
static const int ERR_CODE_BIND_FAILED = -3;
static const int ERR_CODE_LISTEN_FAILED = -4;
static const int ERR_CODE_CLOSE_FAILED = -5;
static const int ERR_CODE_SET_SOCKET_PROP_FAILED = -6;

// ================

int python_register_url(char* path, char* response);
int setup_server(int port_number);
int start_server();

#endif // SERVER_H
