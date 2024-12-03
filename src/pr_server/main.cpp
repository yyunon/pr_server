#include <cstdlib>
#include <memory>
#include <spdlog/spdlog.h>
#include <vector>

#include "listener.hpp"


using namespace http::server;
using namespace std;

int main(int argc, char* argv[]) {
  try
  {
    // Check command line arguments.
    if(argc != 4)
    {
        std::cerr << "Usage: " << argv[0] << " <address> <port>\n";
        std::cerr << "  For IPv4, try:\n";
        std::cerr << "    receiver 0.0.0.0 80\n";
        std::cerr << "  For IPv6, try:\n";
        std::cerr << "    receiver 0::0 80\n";
        return EXIT_FAILURE;
    }

    auto const address = net::ip::make_address(argv[1]);
    unsigned short const port = static_cast<unsigned short>(std::atoi(argv[2]));
    auto const num_threads = std::max<int>(1, static_cast<int>(std::atoi(argv[3])));
    
    std:: cout << "Running server on " << address << ":" << port << " with " << num_threads << " number of threads...\n";

    net::io_context ioc{num_threads};
    std::make_shared<Listener>(
      ioc,
      tcp::endpoint{address, port}
    )->run();

    std::vector<std::thread> thread_vector;
    thread_vector.reserve(static_cast<unsigned long>(num_threads - 1));
    for(auto i = num_threads - 1; i > 0; --i) {
      thread_vector.emplace_back(
        [&ioc] {
          ioc.run();
        }
      );
    }
    ioc.run();
    std::cout << "Server started... \n";
  }
  catch(std::exception const& e)
  {
    std::cerr << "Error: " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
