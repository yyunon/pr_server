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
#include <boost/asio.hpp>
#include <boost/json/parse.hpp>
#include <boost/json/serialize.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/buffers_iterator.hpp>
#include <cstring>
#include <iostream>

namespace http::server {
	namespace beast = boost::beast;
	namespace http = beast::http;
	using tcp = boost::asio::ip::tcp;
  using namespace boost::json;

  struct request_handler {

    http::request<http::dynamic_body> _req;

    explicit request_handler(http::request<http::dynamic_body> req): _req(std::move(req)) {}

    auto bad_request(boost::beast::string_view reason) {
      http::response<http::dynamic_body> result {http::status::bad_request, _req.version()};
      result.set(http::field::content_type, "application/json");
      result.keep_alive(_req.keep_alive());
      object obj ({{"reason", reason}});
      auto serialized_obj = serialize(obj);
      beast::ostream(result.body()) << serialized_obj;
      result.prepare_payload();
      return result;
    }

    auto internal_server_error(boost::beast::string_view reason) {
      http::response<http::dynamic_body> result {http::status::internal_server_error, _req.version()};
      result.set(http::field::content_type, "application/json");
      result.keep_alive(_req.keep_alive());
      object obj ({{"reason", reason}});
      auto serialized_obj = serialize(obj);
      beast::ostream(result.body()) << serialized_obj;
      result.prepare_payload();
      return result;
    }
    
    value parse_request() {
      beast::error_code ec;
      auto buf = boost::asio::buffer_cast<char const*>(beast::buffers_front(_req.body().cdata()));
      value result = parse(beast::string_view(buf), ec);
      if(ec){
        std::cerr << "Parsing failed " << ec.what() <<"\n";
        internal_server_error("Invalid BODY");
      }
      return result;
    }
  };
}
