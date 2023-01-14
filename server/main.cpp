#include <iostream>
#include <fstream>

#include <syslog.h>
#include <unistd.h>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/write.hpp>

using boost::asio::ip::tcp;
using boost::asio::awaitable;
using boost::asio::co_spawn;
using boost::asio::detached;
using boost::asio::use_awaitable;

namespace this_coro = boost::asio::this_coro;

#if defined(BOOST_ASIO_ENABLE_HANDLER_TRACKING)
# define use_awaitable \
  boost::asio::use_awaitable_t(__FILE__, __LINE__, __PRETTY_FUNCTION__)
#endif

awaitable<void> handler(tcp::socket socket) {
    auto endpoint = socket.remote_endpoint();
    auto [address, port] = std::make_tuple(endpoint.address().to_string(), endpoint.port());
    try {
        syslog(LOG_INFO | LOG_USER, "%s:%d connected", address.c_str(), port);

        auto filename = address + "_" + std::to_string(port);
        std::ofstream output(filename, std::fstream::trunc | std::fstream::out);

        std::array<char, 4096> buf{};
        while (true) {
            std::size_t n = co_await socket.async_read_some(boost::asio::buffer(buf), use_awaitable);
            output.write(buf.data(), static_cast<std::streamsize>(n));
            output.flush();
        }
    } catch (std::exception &e) {
        syslog(LOG_INFO | LOG_USER, "%s:%d disconnected", address.c_str(), port);
    }
}

awaitable<void> listener(boost::asio::ip::port_type port) {
    auto executor = co_await this_coro::executor;
    tcp::acceptor acceptor(executor, {tcp::v4(), port});
    while (true) {
        tcp::socket socket = co_await acceptor.async_accept(use_awaitable);
        co_spawn(executor, handler(std::move(socket)), detached);
    }
}

int main(int argc, char *argv[]) {
    try {
        if (argc != 2) {
            std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
            return EXIT_FAILURE;
        }

        boost::asio::io_context io_context;

        boost::asio::signal_set signals(io_context, SIGTERM, SIGHUP);
        signals.async_wait([&](auto, auto) {
            syslog(LOG_INFO | LOG_USER, "Daemon stopped");
            io_context.stop();
        });

        co_spawn(io_context, listener(std::atoi(argv[1])), detached);

        io_context.notify_fork(boost::asio::io_context::fork_prepare);

        if (pid_t pid = fork()) {
            if (pid > 0) {
                exit(EXIT_SUCCESS);
            } else {
                syslog(LOG_ERR | LOG_USER, "First fork failed: %m");
                return EXIT_FAILURE;
            }
        }

        setsid();
        umask(0);

        if (pid_t pid = fork()) {
            if (pid > 0) {
                exit(EXIT_SUCCESS);
            } else {
                syslog(LOG_ERR | LOG_USER, "Second fork failed: %m");
                return EXIT_FAILURE;
            }
        }

        close(0);
        close(1);
        close(2);

        openlog("tcp_server_daemon", LOG_PID, LOG_DAEMON);

        io_context.notify_fork(boost::asio::io_context::fork_child);

        syslog(LOG_INFO | LOG_USER, "Daemon started");

        io_context.run();
    } catch (std::exception &e) {
        syslog(LOG_ERR | LOG_USER, "Exception: %s", e.what());
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}
