#include <iostream>
#include "packet.pb.h"
#include <asio.hpp>
using namespace std;
using asio::ip::tcp;

// client use sync protobuf
class TcpClient {
public:
    TcpClient(asio::io_context& io_context, short port = 13)
        : io_context_(io_context), resolver_(io_context), socket_(io_context) {
        endpoints_ = resolver_.resolve("localhost", std::to_string(port));
    }
    tcp::socket& socket() {
        return socket_;
    }
    void start(){
        asio::connect(socket_, endpoints_);
        cout << "Connected to server." << endl;
        // Create and send a protobuf message
        packet::Packet packet_msg;
        // Serialize the message to a string
        createMessage(packet_msg, 1, "Hello from Client using Protobuf!");
        packet_msg.set_payload("This is the payload data."); // 
        std::string serialized_data;
        if(!packet_msg.SerializePartialToString(&serialized_data)){
            cerr << "Failed to serialize protobuf message." << endl;
            return;
        }
        // prepare header, make sure same to server
        uint32_t type_net = htonl(packet_msg.type());
        uint32_t length_net = htonl(static_cast<uint32_t>(serialized_data.size())); // WARNING: use size()
        memcpy(header, &type_net, 4);
        memcpy(header + 4, &length_net, 4);

        if(sizeof(header) != 8){
            cerr << "Header size mismatch!" << endl;
            return;
        }

        // send header
       asio::write(socket_, asio::buffer(header, sizeof(header)));
        cout << "Sent Header - Type: " << packet_msg.type()
             << ", Length: " << packet_msg.length() << endl;
             // send payload
        asio::write(socket_, asio::buffer(serialized_data));
        cout << "Sent Packet - Type: " << packet_msg.type()
             << ", Length: " << packet_msg.length()
             << ", Note: " << packet_msg.note() << endl;
    }
private:
    tcp::socket socket_;
    asio::io_context& io_context_;
    tcp::resolver resolver_;
    tcp::resolver::results_type endpoints_;
    char header[8]; // 4 bytes type + 4 bytes length

    void createMessage(packet::Packet& packet_msg, uint32_t type,
                       const std::string& note){
        packet_msg.set_type(type);
        packet_msg.set_length(static_cast<uint32_t>(note.size()));
        packet_msg.set_note(note);
    }
};

int main(int argc, char* argv[]){
    if(argc < 1){
        cerr << "Usage: clientP" << endl;
        return 1;
    }

    try{
        asio::io_context io_context;
        TcpClient client(io_context);
        client.start();
    }catch(std::exception& e){
        cerr << "Exception: " << e.what() << endl;
    }
    return 0;
}