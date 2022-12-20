#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <grpcpp/grpcpp.h>

#include "fibo.grpc.pb.h"

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using grpc::ClientReaderWriter;
using fibonachi::FibonachiStore;
using fibonachi::Request;
using fibonachi::Response;

using FibonachiStreamPtr = std::unique_ptr<ClientReaderWriter<Request, Response>>;

class FibonachiStoreClient
{
 public:
  FibonachiStoreClient(std::shared_ptr<Channel> channel)
      : stub_(FibonachiStore::NewStub(channel)) {}

  void
  Exchange(uint32_t n) {
    ClientContext context;
    FibonachiStreamPtr stream = stub_->Calculate(&context);
    Request request;
    request.set_n(n);
    stream->Write(request);

    Response response;
    stream->Read(&response);
    std::cout << n << " = " << response.fib() << "\n";

    stream->WritesDone();
    Status status = stream->Finish();
    if (!status.ok()) {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      std::cout << "RPC failed";
    }
  }

 private:
  std::unique_ptr<FibonachiStore::Stub> stub_;
};

int
main(int argc, char** argv)
{
  // Instantiate the client. It requires a channel, out of which the actual RPCs
  // are created. This channel models a connection to an endpoint (in this case,
  // localhost at port 50051). We indicate that the channel isn't authenticated
  // (use of InsecureChannelCredentials()).
  // In this example, we are using a cache which has been added in as an
  // interceptor.
  FibonachiStoreClient client(grpc::CreateChannel(
    "localhost:50051", grpc::InsecureChannelCredentials()));
  for (uint32_t i = 0; i < 10; ++i)
    client.Exchange(i);
  for (uint32_t i = 0; i < 10; ++i)
    client.Exchange(i);
  return 0;
}
