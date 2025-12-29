#include <asio.hpp>
#include <iostream>
#include <memory>
#include "packet.pb.h"
using namespace std;
using asio::ip::tcp;
// server
class TcpConn : public std::enable_shared_from_this<TcpConn> {
public:
    using TcpConnPtr = std::shared_ptr<TcpConn>;
    static TcpConnPtr create(asio::io_context& io_context){
        return std::make_shared<TcpConn>(io_context);
    }
    TcpConn(asio::io_context& io_context)
        : socket_(io_context) {}

    tcp::socket& socket(){
        return socket_;
    }

    void start(){
        cout << "New client connected." << endl;
        auto self = shared_from_this();
        do_headers();
    }

private:
    tcp::socket socket_;
    char header_buffer_[8]; // 4 bytes type + 4 bytes length
    std::vector<uint8_t> payload_;

    void do_headers(){
        auto self = shared_from_this();
        asio::async_read(socket_, asio::buffer(header_buffer_),
            [this, self](asio::error_code ec, std::size_t /*length*/){
                if(!ec){
                    uint32_t type = ntohl(*reinterpret_cast<uint32_t*>(header_buffer_));
                    uint32_t length = ntohl(*reinterpret_cast<uint32_t*>(header_buffer_ + 4));

                    cout << "Header received - Type: " << type << ", Length: " << length << endl;

                    do_payload(type, length);
                }else {
                    if(ec != asio::error::eof)
                        cerr << "Error on receive headers: " << ec.message() << endl;
                    else
                        cout << "Connection closed by client." << endl;
                }
            });
    }

    void do_payload(uint32_t type, uint32_t length){
        auto self = shared_from_this();
        payload_.resize(length); // resize payload buffer
        asio::async_read(socket_, asio::buffer(payload_.data(), length),
            [this, self, type](asio::error_code ec, std::size_t /*length*/){
                if(!ec){
                    // parse protobuf message
                    packet::Packet packet_msg;
                    cout << "Parsing payload of length: " << payload_.size() << endl;
                    if(packet_msg.ParseFromArray(payload_.data(), static_cast<int>(payload_.size()))){
                        cout << "Parsed Packet - Type: " << packet_msg.type()
                             << ", Length: " << packet_msg.length()
                             << ", Note: " << packet_msg.note() << endl;
                    }else{
                        cerr << "Failed to parse protobuf message." << endl;
                    }
                    // wait for next header
                    do_headers();
                }else{
                    cerr << "Error reading payload: " << ec.message() << endl;
                }

            });
    }
};

class TcpServer {
public:
    TcpServer(asio::io_context& io_context, short port = 13)
    : io_context_(io_context),
    acceptor_(io_context, tcp::endpoint(tcp::v4(), port)) {
        do_accept();
    }
    
private:
    asio::io_context& io_context_;
    tcp::acceptor acceptor_;
    
private:
    void do_accept(){
        try {
            auto new_connection = TcpConn::create(io_context_);
            acceptor_.async_accept(new_connection->socket(),
                [this, new_connection](asio::error_code ec){
                    if(!ec){
                        new_connection->start(); // run TcpConn::Start()
                    }else{
                        cerr << "Accept error: " << ec.message() << endl;
                    }
                    do_accept(); // accept next connection
                });
        } catch (std::exception& e) {
            cerr << "Exception in do_accept: " << e.what() << endl;
        }
    }
};

int main(){
    try{
        asio::io_context io_context;
        TcpServer server(io_context);
        cout << "Server is running on port 13..." << endl;
        io_context.run();
    } catch (std::exception& e){
        cerr << "Exception: " << e.what() << endl;
    }
};