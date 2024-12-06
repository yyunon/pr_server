#include <boost/asio.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/core/bind_handler.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/core/ostream.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/http/dynamic_body.hpp>
#include <boost/beast/http/field.hpp>
#include <boost/beast/http/fields.hpp>
#include <boost/beast/http/impl/write.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/parser.hpp>
#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/verb.hpp>
#include <boost/beast/version.hpp>
#include <boost/json.hpp>
#include <boost/json/serialize.hpp>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <memory>
#include <string>
#include <filesystem>

#include "proto_gen/bigdata.pb.h"
#include "request_handler.hpp"

namespace http::server {
namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;
using namespace boost::json;
using namespace request_handler;

namespace state {
  inline std::size_t request_count()
  {
    static std::size_t count = 0;
    return ++count;
  }
  inline std::time_t now() { return std::time(0); }
}// namespace state

class Server : public std::enable_shared_from_this<Server>
{
  using alloc_t = std::vector<char>::allocator_type;
  using request_body_t = http::string_body;
public:
  explicit Server(tcp::socket &&socket) : _stream(std::move(socket))  {}
  void run()
  {
    net::dispatch(_stream.get_executor(), beast::bind_front_handler(&Server::read_request, shared_from_this()));
  }

private:
  beast::tcp_stream _stream;
  http::request<request_body_t, http::basic_fields<alloc_t>> _request;
  boost::optional<http::request_parser<request_body_t, alloc_t>> _parser;
  alloc_t _alloc;
  beast::flat_buffer _buffer{ 8192 };
  http::response<http::dynamic_body> _response;

  void read_request()
  {
    auto self = shared_from_this();

    _parser.emplace(
      std::piecewise_construct,
      std::make_tuple(),
      std::make_tuple(_alloc)
    );

    http::async_read(_stream, _buffer, *_parser, [self](beast::error_code ec, std::size_t bytes_transferred) {
      boost::ignore_unused(bytes_transferred);
      if (!ec) { 
        self->_request = self->_parser->get();
        self->process_request(); 
        }
    });
  }

  void process_request()
  {
    _stream.expires_after(std::chrono::seconds(120));

    _response.version(_request.version());
    _response.keep_alive(_request.keep_alive());

    switch (_request.method()) {
    case http::verb::get:
      create_response_get();
      break;
    case http::verb::post:
      create_response_post();
      break;
    default:
      _response.result(http::status::bad_request);
      _response.set(http::field::content_type, "text/plain");
      beast::ostream(_response.body()) << "Invalid request-method '" << std::string(_request.method_string()) << "'";
      break;
    }

    write_response();
  }

  void create_response_get()
  {
    auto target = _request.target();

    if (target == "/count") {
      _response.result(http::status::ok);
      _response.set(http::field::server, "YUKSEL");
      _response.set(http::field::content_type, "text/html");
      beast::ostream(_response.body()) << "<html>\n"
                                       << "<head><title>Request count</title></head>\n"
                                       << "<body>\n"
                                       << "<h1>Request count</h1>\n"
                                       << "<p>There have been " << state::request_count() << " requests so far.</p>\n"
                                       << "</body>\n"
                                       << "</html>\n";
    } else if (target == "/time") {
      _response.result(http::status::ok);
      _response.set(http::field::server, "YUKSEL");
      _response.set(http::field::content_type, "text/html");
      beast::ostream(_response.body()) << "<html>\n"
                                       << "<head><title>Current time</title></head>\n"
                                       << "<body>\n"
                                       << "<h1>Current time</h1>\n"
                                       << "<p>The current time is " << state::now() << " seconds since the epoch.</p>\n"
                                       << "</body>\n"
                                       << "</html>\n";
    } else {
      _response = request_handler::bad_request(_request, "Wrong path!");
    }
  }

  void create_response_post()
  {
    auto parsed_body = request_handler::parse_request(_request);

    if (parsed_body == nullptr) {
      _response = request_handler::bad_request(_request, "Incorrect body");
      return;
    }

    auto path = parsed_body.at("path").as_string();
    path.append(".dat");
    namespace fs = std::filesystem;
    const fs::path db{path.c_str()};
    if(!fs::exists(db)){
      _response = request_handler::internal_server_error(_request, "No such db exists.");
      return;
    }
    auto target = _request.target();

    std::ifstream bigFile(db);
    std::string data;
    constexpr size_t buffer_size = 1024 * 1024;
    std::unique_ptr<char[]> buffer(new char[buffer_size]);

    while (bigFile.read(buffer.get(), buffer_size)) { data += buffer.get(); }
    if (target == "/bigFileJson") {
      object obj({ { "value", data } });
      beast::ostream(_response.body()) << obj;
    } else if (target == "/bigFileProto") {
      bigdata::BigData dat;
      dat.set_value(data);
      auto outbeast = beast::ostream(_response.body());
      dat.SerializeToOstream(&outbeast);

    } else {
      _response = request_handler::bad_request(_request, "Wrong path!");
    }
  }

  void write_response()
  {
    auto self = shared_from_this();
    _response.content_length(_response.body().size());
    http::async_write(_stream, _response, [self](beast::error_code ec, std::size_t) {
      self->_stream.socket().shutdown(tcp::socket::shutdown_send, ec);
    });
  }
};
}// namespace http::server
