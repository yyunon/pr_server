#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
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
#include <cstdlib>
#include <ctime>
#include <memory>

#include "server.hpp"
#include "utils.hpp"

namespace beast = boost::beast;
namespace net = boost::asio;
using tcp = boost::asio::ip::tcp;

class Listener: public std::enable_shared_from_this<Listener> {
  net::io_context& _ioc;
  tcp::acceptor _acceptor;

public:
  Listener(net::io_context& ioc, tcp::endpoint endpoint): _ioc(ioc), _acceptor(net::make_strand(ioc)) {
    beast::error_code ec;

    _acceptor.open(endpoint.protocol(), ec);
    if(ec) {
      fail(ec, "open");
      return;
    }

    _acceptor.set_option(net::socket_base::reuse_address(true), ec);
    if(ec) {
      fail(ec, "set_option");
      return;
    }
    
    _acceptor.bind(endpoint, ec);
    if(ec) {
      fail(ec, "bind");
      return;
    }

    _acceptor.listen(net::socket_base::max_listen_connections, ec);
    if(ec) {
      fail(ec, "listen");
      return;
    }
  }
    
  void run() {
    do_accept();
  }

private:
  void do_accept() {
    _acceptor.async_accept(
      net::make_strand(_ioc),
      beast::bind_front_handler(&Listener::on_accept,shared_from_this())
    );
  }

  void on_accept(beast::error_code ec, tcp::socket socket) {
    if(ec) {
      fail(ec, "accept");
      return;
    } else {
      std::make_shared<http::server::Server>(std::move(socket))->run();
    }
    do_accept();
  }

};
