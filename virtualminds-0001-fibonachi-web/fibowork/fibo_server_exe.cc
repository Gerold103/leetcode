#include "fibo_server.h"

#include <cstring>
#include <iostream>

int
main(int argc, char** argv)
{
	if (argc < 2) {
		std::cout << "Expected a port arg to listen on"
			  << std::endl;
		return -1;
	}
	const int buf_size = 128;
	char host[buf_size];
	int rc = snprintf(host, buf_size, "0.0.0.0:%s", argv[1]);
	if (rc < 0 || rc == buf_size) {
		std::cout << "Invalid port" << std::endl;
		return -1;
	}
	FibonacciServerListen(host);
	return 0;
}