#include <boost/beast/core/error.hpp>
#include <iostream>

namespace beast = boost::beast;

void fail(beast::error_code ec, char const* what){
  std::cerr << what << ":" << ec.message() << "\n";
}
