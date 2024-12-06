#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/buffers_iterator.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/core/bind_handler.hpp>
#include <boost/beast/core/buffers_prefix.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/core/multi_buffer.hpp>
#include <boost/beast/core/ostream.hpp>
#include <boost/beast/core/string_type.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/http/basic_dynamic_body.hpp>
#include <boost/beast/http/dynamic_body.hpp>
#include <boost/beast/http/field.hpp>
#include <boost/beast/http/impl/write.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/version.hpp>
#include <boost/json/parse.hpp>
#include <boost/json/serialize.hpp>
#include <cstddef>
#include <iostream>
#include <new>
#include <string>

namespace http::server {
namespace beast = boost::beast;
namespace http = beast::http;
using tcp = boost::asio::ip::tcp;
using namespace boost::json;

namespace request_handler
{
  using alloc_t = std::vector<char>::allocator_type;
  using request_body_t = http::string_body;
  using request_t = http::request<request_body_t, http::basic_fields<alloc_t>>;
  auto bad_request(request_t _req, boost::beast::string_view reason)
  {
    http::response<http::dynamic_body> result{ http::status::bad_request, _req.version() };
    result.set(http::field::content_type, "application/json");
    result.keep_alive(_req.keep_alive());
    object obj({ { "reason", reason } });
    auto serialized_obj = serialize(obj);
    beast::ostream(result.body()) << serialized_obj;
    result.prepare_payload();
    return result;
  }

  auto internal_server_error(request_t _req, boost::beast::string_view reason)
  {
    http::response<http::dynamic_body> result{ http::status::internal_server_error, _req.version() };
    result.set(http::field::content_type, "application/json");
    result.keep_alive(_req.keep_alive());
    object obj({ { "reason", reason } });
    auto serialized_obj = serialize(obj);
    beast::ostream(result.body()) << serialized_obj;
    result.prepare_payload();
    return result;
  }

  value parse_request(request_t _request)
  {
    try {
      beast::error_code ec;
      value result = parse(_request.body(), ec);
      if (ec) {
        std::cerr << "Parsing failed " << ec.what() << "\n";
        return nullptr;
      }
      return result;

    } catch (std::bad_alloc ec) {
      std::cerr << "Parsing failed " << ec.what() << "\n";
      return nullptr;
    }
  }
};
}// namespace http::server
