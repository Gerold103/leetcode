#pragma once

#include <cstdint>

class FibonacciStoreClient;

class FibonacciClient
{
public:
	FibonacciClient(const char* addr);
	~FibonacciClient();

	// Calculate n-th value of Fibonacci number.
	void Calculate(uint32_t n);

private:
	FibonacciStoreClient* m_impl;
};
