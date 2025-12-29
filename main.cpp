#include <iostream>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <vector>
using namespace std;

struct Data {
    uint32_t id;
    float value;
};

int main(){
    
    // Create a TCP socket
   int server_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    // check if failed
    if (server_fd == -1){
        cerr << "Socket creation failed." << endl;
        return 1; 
    } 

    // Define Server Address Structure
    sockaddr_in address; // socket address structure
    address.sin_family = AF_INET; // IPv4
    address.sin_addr.s_addr = INADDR_ANY; // bind 0.0.0.0
    address.sin_port = htons(8080); // port 8080 in network byte order

    // Bind the socket to the specified IP and port
    int bindRes = ::bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    if (bindRes < 0){
        perror("Bind Failed");
        return 1;
    }
    cout << "Socket successfully created and bound to port 8080." << endl;
    
    // listen for incoming connections
    if(::listen(server_fd, 1) < 0){ 
        perror("Listen failed.");
        return 1;
    }
    cout << "Listening for incoming connections..." << endl;

    // accept a connection (this will block until a connection is made)
    int client_fd = ::accept(server_fd, nullptr, nullptr);
    // if accept failed
    if(client_fd < 0){
        perror("Accept failed.");
        return 1;
    }
    cout << "Connection accepted." << endl;

    // recvive data from the client
    char buffer[1024];
    size_t bytes_recv = ::recv(client_fd, buffer, sizeof(buffer) - 1, 0);
    // check if recv failed
    if(bytes_recv < 0){
        perror("Receive failed.");
        return 1;
    }

    // null-terminate and print the received message
    buffer[bytes_recv] = '\0'; // add null terminator to make it a valid string
    cout << "Received message: " << buffer << endl;

    // send a response to client
    const char* reply = "Message received loud and clear.";
    ::send(client_fd, reply, ::strlen(reply), 0);

    ::close(client_fd); // close client socket
    ::close(server_fd); // close server socket

    return 0;
} 