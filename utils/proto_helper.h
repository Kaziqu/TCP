#ifndef PROTO_HELPER_H
#define PROTO_HELPER_H

#include <string>
#include <google/protobuf/message.h>
#include <asio.hpp>
using namespace std;
using asio::ip::tcp;
using google::protobuf::Message;

void send_protobuf_message(asio::io_context& io, tcp::socket& socket, const Message& message, asio::error_code& ec){
    // Serialize protobuf message to string
    auto serialized = std::make_shared<std::string>();
    // check serialization success
    if(!message.SerializeToString(serialized.get())){
        cerr << "Failed to serialize protobuf message." << endl;
        ec = asio::error::operation_aborted; // set some error code
        return;
    }
    if(serialized->empty() || serialized->size() > 65535){
        cerr << "Serialized protobuf message is empty or too large." << endl;
        ec = asio::error::operation_aborted; // set some error code
        return;
    }

    uint16_t len = static_cast<uint16_t>(serialized->size()); // cut to 2 bytes
    uint16_t len_net = htons(len);   // convert to network byte order
    cout << "Sending message of length: " << len << endl;

    // validate length
    if(len > 65535){ // length of uint16_t
        cerr << "Message too large to send." << endl;
        ec = asio::error::message_size; // set some error code
        return;
    }
    const uint8_t* data_ptr = reinterpret_cast<const uint8_t*>(serialized->data());
    // Create buffer with length prefix
    auto buffer = std::make_shared<std::vector<uint8_t>>();
    buffer->insert(buffer->end(),
                  reinterpret_cast<const uint8_t*>(&len_net),
                  reinterpret_cast<const uint8_t*>(&len_net) + sizeof(len_net));
    buffer->insert(buffer->end(),
                  serialized->begin(),
                  serialized->end());

    // Send the buffer
    asio::async_write(socket, asio::buffer(*buffer),
        [buffer, serialized, &ec](asio::error_code ec, std::size_t /*length*/){
            if(ec){
                cerr << "Error sending protobuf message: " << ec.message() << endl;
                ec = ec;
            }else{
                cout << "Protobuf message sent successfully." << endl;
            }
        });
}


void recv_protobuf_message(tcp::socket& socket, Message& message, asio::error_code& ec){
    // read length prefix
    uint16_t len_net;
    asio::async_read(socket, asio::buffer(&len_net, sizeof(len_net)),
        [&socket, &message, &ec, len_net](asio::error_code ec, std::size_t /*length*/){
            if(!ec){
                uint16_t len = ntohs(len_net);
                cout << "Receiving message of length: " << len << endl;
                // read the protobuf message based on length
                auto buffer = std::make_shared<std::vector<uint8_t>>(len);
                asio::async_read(socket, asio::buffer(*buffer),
                    [buffer, &message, &ec](asio::error_code ec, std::size_t /*length*/){
                        if(!ec){
                            // parse the protobuf message
                            if(!message.ParseFromArray(buffer->data(), static_cast<int>(buffer->size()))){
                                cerr << "Failed to parse protobuf message." << endl;
                                ec = asio::error::operation_aborted; // set some error code
                            }else{
                                cout << "Protobuf message received successfully." << endl;
                            }
                        }else{
                            cerr << "Error receiving protobuf message: " << ec.message() << endl;
                            ec = ec;
                        }
                    });
            }else{
                cerr << "Error reading message length: " << ec.message() << endl;
                ec = ec;
            }
        });
        }

#endif // PROTO_HELPER_H

