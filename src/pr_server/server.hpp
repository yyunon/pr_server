#include <boost/asio/dispatch.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/core/bind_handler.hpp>
#include <boost/beast/core/error.hpp>
#include <boost/beast/core/ostream.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/http/dynamic_body.hpp>
#include <boost/beast/http/field.hpp>
#include <boost/beast/http/impl/write.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio.hpp>
#include <boost/json/serialize.hpp>
#include <fstream>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <memory>
#include <string>
#include <boost/json.hpp>

#include "proto_gen/bigdata.pb.h"

namespace http::server {
	namespace beast = boost::beast;
	namespace http = beast::http;
	namespace net = boost::asio;
	using tcp = boost::asio::ip::tcp;
  using namespace boost::json;

	namespace state {
		inline std::size_t request_count() {
			static std::size_t count = 0;
			return ++count;
		}
		inline std::time_t now() {
			return std::time(0);
		}
	}

	class Server: public std::enable_shared_from_this<Server> {
		public:	
			explicit Server(tcp::socket&& socket): _stream(std::move(socket)){}
			void run() {
        net::dispatch(_stream.get_executor(),
                      beast::bind_front_handler(&Server::read_request, shared_from_this()));
			}

		private:
      beast::tcp_stream _stream;
			beast::flat_buffer _buffer{8192};
			http::request<http::dynamic_body> _request;
			http::response<http::dynamic_body> _response;

			void read_request() {
				auto self = shared_from_this();
				http::async_read(
					_stream,
					_buffer,
					_request,
					[self](beast::error_code ec, std::size_t bytes_transferred) {
						boost::ignore_unused(bytes_transferred);
						if(!ec) {
							self->process_request();
						}
					}
				);
			}

			void process_request(){
        _stream.expires_after(std::chrono::seconds(120));

				_response.version(_request.version());
				_response.keep_alive(false);

				switch(_request.method()) {
					case http::verb::get:
						_response.result(http::status::ok);
						_response.set(http::field::server, "YUKSEL");
						create_response();
						break;
					default:
						_response.result(http::status::bad_request);
						_response.set(http::field::content_type, "text/plain");
						beast::ostream(_response.body())
							<< "Invalid request-method '"
							<< std::string(_request.method_string())
							<< "'";
						break;
				}

				write_response();
			}

			void create_response() {
				auto target = _request.target();
				if(target == "/count") {
					_response.set(http::field::content_type, "text/html");
					beast::ostream(_response.body())
						<< "<html>\n"
						<<  "<head><title>Request count</title></head>\n"
						<<  "<body>\n"
						<<  "<h1>Request count</h1>\n"
						<<  "<p>There have been "
						<<  state::request_count()
						<<  " requests so far.</p>\n"
						<<  "</body>\n"
						<<  "</html>\n";
				} else if (target == "/time") {
					_response.set(http::field::content_type, "text/html");
					beast::ostream(_response.body())
						<<  "<html>\n"
						<<  "<head><title>Current time</title></head>\n"
						<<  "<body>\n"
						<<  "<h1>Current time</h1>\n"
						<<  "<p>The current time is "
						<<  state::now()
						<<  " seconds since the epoch.</p>\n"
						<<  "</body>\n"
						<<  "</html>\n";	
    } else if (target == "/bigFileJSON") {
          std::ifstream bigFile("/data/workspace/data/bigFile.dat");
          constexpr size_t buffer_size = 1024 * 1024;
          std::unique_ptr<char[]> buffer(new char[buffer_size]);
          bigFile.read(buffer.get(), buffer_size);
          object obj;
          obj["value"] = buffer.get();
          beast::ostream(_response.body())
            << obj;
    } else if (target == "/bigFileProto") {
          std::ifstream bigFile("/data/workspace/data/bigFile.dat");
          constexpr size_t buffer_size = 1024 * 1024;
          std::unique_ptr<char[]> buffer(new char[buffer_size]);
          bigFile.read(buffer.get(), buffer_size);
          bigdata::BigData dat;
          dat.set_value(buffer.get());
          auto outbeast = beast::ostream(_response.body());
          dat.SerializeToOstream(&outbeast);

    } else {
					_response.result(http::status::not_found);
					_response.set(http::field::content_type, "text/plain");
					beast::ostream(_response.body()) << "File not found\r\n";

    }
  }

			void write_response() {
				auto self = shared_from_this();
				_response.content_length(_response.body().size());
				http::async_write(
					_stream,
					_response,
					[self](beast::error_code ec, std::size_t) {
						self->_stream.socket().shutdown(tcp::socket::shutdown_send, ec);
					}
				);

			}

	};
}
