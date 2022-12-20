#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <grpcpp/grpcpp.h>

#include "fibo.grpc.pb.h"

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::ServerReaderWriter;
using grpc::Status;
using fibonachi::FibonachiStore;
using fibonachi::Request;
using fibonachi::Response;

struct FibocachiPair
{
  uint64_t m_result = 0;
  uint64_t m_count = 0;
};

class FibonachiStoreServiceImpl final
  : public FibonachiStore::Service {

public:
  FibonachiStoreServiceImpl()
  {
    FibocachiPair pair;
    m_cache.push_back(pair);
    pair.m_result = 1;
    m_cache.push_back(pair);
  }

private:
  Status
  Calculate(ServerContext* context,
            ServerReaderWriter<Response, Request>* stream) override
  {
    Request request;
    while (stream->Read(&request)) {
      uint32_t n = request.n();
      Response response;

      m_mutex.lock();
      if (n < m_cache.size()) {
        FibocachiPair& pair = m_cache[n];
        uint64_t count = ++pair.m_count;
        uint64_t res = pair.m_result;
        m_mutex.unlock();

        std::cout << "Cache hit\n";
        response.set_fib(res);
        response.set_count(count);
      } else {
        assert(n >= 2);
        FibocachiPair pair;
        pair.m_count = 1;
        uint64_t start_n = m_cache.size();
        m_cache.reserve(n + 1);
        for (uint64_t i = m_cache.size(); i <= n; ++i) {
          pair.m_result = m_cache[i - 1].m_result + m_cache[i - 2].m_result;
          m_cache.push_back(pair);
        }
        uint64_t res = m_cache[n].m_result;
        m_mutex.unlock();

        response.set_fib(res);
        response.set_count(1);
      }
      response.set_timestamp(time(nullptr));
      stream->Write(response);
    }
    return Status::OK;
  }

  std::mutex m_mutex;
  std::vector<FibocachiPair> m_cache;
};

void
RunServer()
{
  std::string server_address("0.0.0.0:50051");
  FibonachiStoreServiceImpl service;

  ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case, it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  // Finally assemble the server.
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}

int
main(int argc, char** argv)
{
  RunServer();

  return 0;
}
