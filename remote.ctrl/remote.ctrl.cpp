#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <iostream>
#include <string>

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
using tcp = net::ip::tcp;

// Global variable to store the state
int variavel = 0;

void on_message(websocket::stream<tcp::socket>& ws, beast::error_code ec, beast::flat_buffer& buffer) {
    if (ec) {
        std::cerr << "Error: " << ec.message() << std::endl;
        return;
    }

    // Read the message from the client
    std::string message = beast::buffers_to_string(buffer.data());
    std::cout << "Received: " << message << std::endl;

    // Update the "variavel" variable based on the message
    if (message == "set_0") {
        variavel = 0;
    }
    else if (message == "set_1") {
        variavel = 1;
    }

    // Send a response back to the client
    std::string response = "variavel: " + std::to_string(variavel);
    ws.text(true);
    ws.write(net::buffer(response));

    // Clear the buffer for the next message
    buffer.consume(buffer.size());

    // Listen for the next message
    ws.async_read(buffer, [&ws, &buffer](beast::error_code ec, std::size_t) {
        on_message(ws, ec, buffer);
        });
}

int main() {
    try {
        net::io_context io_context;

        // Create a TCP acceptor to listen for incoming connections
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 8080));
        std::cout << "WebSocket server listening on port 8080..." << std::endl;

        while (true) {
            // Accept a new connection
            tcp::socket socket(io_context);
            acceptor.accept(socket);

            // Create a WebSocket stream and accept the connection
            websocket::stream<tcp::socket> ws(std::move(socket));
            ws.accept();

            std::cout << "Client connected!" << std::endl;

            // Start reading messages
            beast::flat_buffer buffer;
            ws.async_read(buffer, [&ws, &buffer](beast::error_code ec, std::size_t) {
                on_message(ws, ec, buffer);
                });

            // Run the IO context
            io_context.run();
        }
    }
    catch (std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}