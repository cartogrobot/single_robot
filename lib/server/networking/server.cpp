/*
 * server.cpp
 * Author: Aven Bross
 * Date: 8/20/2015
 * 
 * Description:
 * Multithreaded server to recieve and manage connections.
*/

#ifndef __SERVER_CPP
#define __SERVER_CPP

#include "server.h"

// Set up socket abort to not exit or print
static int server_skt_abort(int code,const char *msg){
    return -1;
}


/*
 * class Server
 * Virtual server class representing a multithreaded server
 */

// Constructor
Server::Server(unsigned int port): _dead(true){
    skt_set_abort(server_skt_abort);
    _socket = skt_server(&port);
}

// Start server loop
void Server::start(){
    if(_dead){
        _dead = false;
        std::thread temp(&Server::loop, this);
        std::swap(_thread,temp);
    }
}

// Stop server loop
void Server::stop(){
    if(!_dead){
        _dead = true;
        _thread.join();
        std::thread temp;
        std::swap(_thread,temp);
        _thread.join();
        _connections.clear();
    }
}

// Kill a connection by it's address
void Server::kill(const sockaddr & connectionAddress){
    std::unique_lock<std::mutex> connectionLock(_mutex, std::defer_lock);
    connectionLock.lock();
    _connections.erase(to_string(connectionAddress));
    connectionLock.unlock();
}

// Stop server loop
bool Server::isDead(){
    return _dead;
}

// Destructor
Server::~Server() {
    stop();
}


/*
 * class TCPServer : Server
 * Sublcass of server representing a TCP server
 */

// Creates a new connection for the given socket and address
std::shared_ptr<TCPConnection> TCPServer::makeConnection(int socket, const sockaddr & clientAddress){
    return std::make_shared<TCPConnection>(socket, this, clientAddress);
}

// Server loop to handle incoming connections
void TCPServer::loop(){
    std::unique_lock<std::mutex> connectionLock(_mutex, std::defer_lock);
    while(!_dead){
        skt_ip_t client_ip;      // IP & port of other end of connection
        unsigned int client_port;
        
        int cSocket = skt_accept(_socket, &client_ip, &client_port);
        sockaddr_in address = skt_build_addr(client_ip, client_port);
        sockaddr client_addr = *((sockaddr *)(&address));
        
        connectionLock.lock();
        std::shared_ptr<TCPConnection> newCon = makeConnection(cSocket, client_addr);
        newCon -> start();
        _connections[to_string(client_addr)] = newCon;
        connectionLock.unlock();
    }
}


/*
 * class UDPServer : Server
 * Sublcass of server representing a UDP server
 */
 
// Creates a new connection for the given socket and address
std::shared_ptr<UDPConnection> UDPServer::makeConnection(int socket, const sockaddr & clientAddress){
    return std::make_shared<UDPConnection>(socket, this, clientAddress);
}
 
// Server loop to handle incoming messages
void UDPServer::loop(){
    std::unique_lock<std::mutex> connectionLock(_mutex, std::defer_lock);
    sockaddr client_addr;      // IP & port of other end of connection
    unsigned int client_size;
    char c; 
    std::string str="";
    const char *term=" \t\r\n";
    while (!_dead) {
	    if(skt_recvN_from(_socket,&c,1,&client_addr, &client_size) != 0){
	        stop(); // Stop on error
	    }
	    if (strchr(term,c)) {
		    if (c=='\r') continue; /* will be CR/LF; wait for LF */
		    else{
		        connectionLock.lock();
		        
		        // Create connection if needed
		        if(_connections.count(to_string(client_addr)) == 0){
		            std::shared_ptr<UDPConnection> newCon = makeConnection(_socket, client_addr);
		            newCon -> start();
		            _connections[to_string(client_addr)] = newCon;
		        }
		        
		        // Push the message to the connection for handling
		        ((UDPConnection*)(_connections[to_string(client_addr)].get())) -> push(str);
		        
		        connectionLock.unlock();
		    }
	    }
	    else str+=c; /* normal character--add to string and continue */
    }
}


// Converts sockaddr to string for hashing and comparison
std::string to_string(const sockaddr & addr){
    int size = sizeof(sockaddr);
    char bitstring[size];
    std::memcpy(bitstring, &addr, sizeof(sockaddr));
    return std::string(bitstring, size);
}


/*
 * class Connection
 * Connection class representing a connection to remote host
 */

// Constructor takes ptr to message handler
Connection::Connection(int socket, Server * server, const sockaddr & toAddress): _server(server), 
  _socket(socket) {
    std::memcpy(&_toAddress, &toAddress, sizeof(sockaddr));
}

// Start the connection loop
void Connection::start(){
    _thread = std::thread(&Connection::loop, this);
    _thread.detach();
}

// Kills this connection
void Connection::kill(){
    // Call the server kill function on this address
    _server -> kill(_toAddress);
}

// Called on connection creation
void Connection::onOpen(){
    // Do nothing, overload in subclasses
}

// Called when new message is recieved
void Connection::onMessage(const std::string & message){
    std::cout << "socket: " << _socket << " sent: " << message << "\n";
}

// Called on connection closure
void Connection::onClose(){
    // Do nothing, overload in subclasses
}

// Declaring pure virtual destructor
Connection::~Connection(){
    _dead = true;
}


/*
 * class TCPConnection : Connection
 * Connection class representing a TCP connection to remote host
 */

// Constructor takes ptr to message handler
TCPConnection::TCPConnection(int socket, Server * server, const sockaddr & toAddress): 
  Connection(socket, server, toAddress) {
    std::cout << "opening socket: " << _socket << "\n";
}

// Connection destructor
TCPConnection::~TCPConnection(){
    std::cout << "closing socket: " << _socket << "\n";
    _dead = true;
    skt_close(_socket);
}

// Send message to connection recipient, blocks waiting for send
bool TCPConnection::sendMessage(const std::string & message){
    if(skt_sendN(_socket, message.c_str(), message.size()+1) != 0){
        fail();
        return false;
    }
    return true;
}

// Loop to handle connection and recieve messages
void TCPConnection::loop(){
    const char *term=" \t\r\n";
    std::string message = "";
    char c; 
    
    onOpen();
    
    while(!_dead){
	    if(skt_recvN(_socket,&c,1) != 0){
	        fail(); // Connection failure
	    }
	    else if(strchr(term,c)){
		    if(c=='\r') continue; // will be CR/LF; wait for LF
		    onMessage(message);
		    message = "";
	    }
	    else{
	        message+=c; // normal character
        }
    }
}

// Connection failure routine
void TCPConnection::fail(){
    _dead = true;
    onClose();
    kill();
    std::cout << "failed socket: " << _socket << "\n";
}


/*
 * class UDPConnection : Connection
 * Connection class representing a UDP connection to remote host
 */

// Create and detach the thread to handle this address
UDPConnection::UDPConnection(int socket, Server * server, const sockaddr & toAddress): 
  Connection(socket, server, toAddress){
    std::cout << "connection recieved\n";
}

// Called to push messages to this connection
void UDPConnection::push(const std::string & message){
    std::unique_lock<std::mutex> recieveLock(_messageMutex);
    recieveLock.lock();
    _messageQueue.push(message);
}

// Send message to connection recipient, blocks waiting for send
bool UDPConnection::sendMessage(const std::string & message){
    if(skt_sendN_to(_socket, message.c_str(), message.size()+1,
                    &_toAddress, sizeof(_toAddress)) != 0){
        fail();
        return false;
    }
    return true;
}

// Connection destructor
UDPConnection::~UDPConnection(){
    _dead = true;
    _cv.notify_all();
    onClose();
    std::cout << "connection dead\n";
}

// Loop to handle connection and recieve messages
void UDPConnection::loop(){
    std::unique_lock<std::mutex> recieveLock(_messageMutex, std::defer_lock);
    
    onOpen();
    
    while(!_dead){
        while(_messageQueue.size() > 0){
            onMessage(_messageQueue.front());
            recieveLock.lock();
            _messageQueue.pop();
            recieveLock.unlock();
        }
        _cv.wait(recieveLock);
        recieveLock.unlock();
    }
}

// Connection failure routine
void UDPConnection::fail(){
    std::cout << "Connection failed\n";
    kill();
}

#endif
