#include "fibo_server.h"
#include "fibo.grpc.pb.h"

#include <iostream>
#include <memory>
#include <vector>
#include <grpcpp/grpcpp.h>

using FibonacciStreamPtr = grpc::ServerReaderWriter<
	fibonacci::Response, fibonacci::Request>;

struct FibonacciPair
{
	uint64_t m_result = 0;
	uint64_t m_count = 0;
};

class FibonacciStoreServiceImpl final
	: public fibonacci::Store::Service
{
public:
	FibonacciStoreServiceImpl();

private:
	grpc::Status
	Calculate(grpc::ServerContext* context,
		  FibonacciStreamPtr* stream) override;

	std::mutex m_mutex;
	std::vector<FibonacciPair> m_cache;
};

////////////////////////////////////////////////////////////////////////////////

FibonacciStoreServiceImpl::FibonacciStoreServiceImpl()
{
	// Basis for all following numbers. These are predefined.
	FibonacciPair pair;
	m_cache.push_back(pair);
	pair.m_result = 1;
	m_cache.push_back(pair);
}

grpc::Status
FibonacciStoreServiceImpl::Calculate(grpc::ServerContext* context,
				     FibonacciStreamPtr* stream)
{
	fibonacci::Request request;
	while (stream->Read(&request)) {
		uint32_t n = request.n();
		fibonacci::Response response;

		m_mutex.lock();
		if (n < m_cache.size()) {
			FibonacciPair& pair = m_cache[n];
			uint64_t count = ++pair.m_count;
			uint64_t res = pair.m_result;
			m_mutex.unlock();

			response.set_fib(res);
			response.set_count(count);
		} else {
			assert(n >= 2);
			FibonacciPair pair;
			pair.m_count = 0;
			uint64_t start_n = m_cache.size();
			m_cache.reserve(n + 1);
			for (uint64_t i = m_cache.size(); i <= n; ++i) {
				uint64_t v1 = m_cache[i - 1].m_result;
				uint64_t v2 = m_cache[i - 2].m_result;
				if (v1 > UINT64_MAX - v2) {
					m_mutex.unlock();
					return grpc::Status(
						grpc::OUT_OF_RANGE,
						"too big number");
				}
				pair.m_result = v1 + v2;
				m_cache.push_back(pair);
			}
			m_cache[n].m_count = 1;
			uint64_t res = m_cache[n].m_result;
			m_mutex.unlock();

			response.set_fib(res);
			response.set_count(1);
		}
		response.set_timestamp(time(nullptr));
		stream->Write(response);
	}
	return grpc::Status::OK;
}

////////////////////////////////////////////////////////////////////////////////

void
FibonacciServerListen(const char *addr)
{
	FibonacciStoreServiceImpl service;
	grpc::ServerBuilder builder;
	builder.AddListeningPort(addr, grpc::InsecureServerCredentials());
	builder.RegisterService(&service);
	std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
	std::cout << "Server listening on " << addr << std::endl;
	server->Wait();
}
