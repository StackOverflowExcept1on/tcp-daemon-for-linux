#include <iostream>
#include <fstream>

#include <boost/asio.hpp>

using boost::asio::ip::tcp;

int main(int argc, char *argv[]) {
    try {
        if (argc != 4) {
            std::cerr << "Usage: " << argv[0] << " <host> <port> <file>" << std::endl;
            return EXIT_FAILURE;
        }

        auto filename = argv[3];
        std::ifstream stream(filename, std::ifstream::binary);

        if (!stream.is_open()) {
            std::cerr << "Can't open file " << filename << std::endl;
            return EXIT_FAILURE;
        }

        boost::asio::io_context io_context;

        tcp::resolver resolver(io_context);
        auto endpoint = resolver.resolve(argv[1], argv[2]);

        tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoint);

        std::array<char, 4096> buf{};
        while (!stream.eof()) {
            stream.read(buf.data(), buf.size());
            boost::asio::write(socket, boost::asio::buffer(buf.data(), stream.gcount()));
        }
    } catch (std::exception &e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return EXIT_SUCCESS;
}
