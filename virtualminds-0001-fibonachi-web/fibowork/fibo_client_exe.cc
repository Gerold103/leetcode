#include "fibo_client.h"

#include <iostream>

int
main(int argc, char** argv)
{
	if (argc < 2) {
		std::cout << "Expected an address arg to connect to" << std::endl;
		return -1;
	}
	FibonacciClient client(argv[1]);
	uint32_t n = 0;
	while (std::cin >> n)
		client.Calculate(n);
	return 0;
}