#include "fibo_client.h"
#include "fibo.grpc.pb.h"

#include <iostream>
#include <memory>
#include <grpcpp/grpcpp.h>

using FibonacciStreamPtr = std::unique_ptr<grpc::ClientReaderWriter<
	fibonacci::Request, fibonacci::Response>>;

class FibonacciStoreClient
{
 public:
	FibonacciStoreClient(std::shared_ptr<grpc::Channel> channel);

	void
	Calculate(uint32_t n);

 private:
	std::unique_ptr<fibonacci::Store::Stub> m_stub;
};

////////////////////////////////////////////////////////////////////////////////

FibonacciClient::FibonacciClient(const char* addr)
	: m_impl(new FibonacciStoreClient(grpc::CreateChannel(
		addr, grpc::InsecureChannelCredentials())))
{
}

FibonacciClient::~FibonacciClient()
{
	delete m_impl;
}

void
FibonacciClient::Calculate(uint32_t n)
{
	m_impl->Calculate(n);
}

////////////////////////////////////////////////////////////////////////////////

FibonacciStoreClient::FibonacciStoreClient(
	std::shared_ptr<grpc::Channel> channel)
	: m_stub(fibonacci::Store::NewStub(channel))
{
}

void
FibonacciStoreClient::Calculate(uint32_t n)
{
	grpc::ClientContext context;
	FibonacciStreamPtr stream = m_stub->Calculate(&context);
	fibonacci::Request request;
	request.set_n(n);
	stream->Write(request);

	fibonacci::Response response;
	stream->Read(&response);
	stream->WritesDone();
	grpc::Status status = stream->Finish();
	if (!status.ok()) {
		grpc::StatusCode code = status.error_code();
		std::cout << "Error: " << code << ": "
			  << status.error_message() << std::endl;
		if (code == grpc::OUT_OF_RANGE)
			return;
		// Unknown error, better crash than undefined behaviour.
		abort();
	}

	std::cout << n << " = {fib = " << response.fib()
		  << ", ts = " << response.timestamp()
		  << ", count = " << response.count() << "}" << std::endl;
}
