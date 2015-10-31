/*
 * websocket_server.cpp
 * Author: Aven Bross
 * Date: 8/21/2015
 * 
 * Description:
 * Multithreaded websocket server.
*/

#include "websocket_server.h"

/*
 * class WebSocketServer : TCPServer
 * Subclass of TCPServer that recieves and handles websocket connections
 */

// Make a new connection for the server
std::shared_ptr<TCPConnection> WebSocketServer::makeConnection(int socket, const sockaddr & clientAddress){
    return std::make_shared<WebSocketConnection>(socket, this, clientAddress);
}


/*
 * class WebSocketConnection : TCPConnection
 * Connection class representing a WebSocket connection to remote host
 */

// Magic string constant for finding handshake keys
const std::string WebSocketConnection::_magicString = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

// Constructor, does any initializations necessary then calls parent constructor
WebSocketConnection::WebSocketConnection(int socket, Server * server, const sockaddr & toAddress):
  TCPConnection(socket, server, toAddress), _handshake(false) {}                                   
 
// Send message via websocket protocol
bool WebSocketConnection::sendMessage(const std::string & message, bool binary){
    if(!_handshake){
        // No handshake, cannot send via websocket
        return false;
    }
    
    unsigned char opcode = binary ? 0x2 : 0x1;
    
    unsigned int bytesSent = 0;
    while(bytesSent < message.size() && !_dead){
        if(bytesSent == 0){
            if(message.size() > USHRT_MAX){
                if(!sendFrame(false, opcode, message.substr(0, USHRT_MAX))){
                    return false;
                }
                bytesSent += USHRT_MAX;
            }
            else{
                if(!sendFrame(true, opcode, message)){
                    return false;
                }
                bytesSent += message.size();
            }
        }
        else{
            unsigned int bytesLeft = message.size() - bytesSent;
            if(bytesLeft > USHRT_MAX){
                if(!sendFrame(false, (unsigned char)(0x0), message.substr(bytesSent, bytesSent + USHRT_MAX))){
                    return false;
                }
                bytesSent += USHRT_MAX;
            }
            else{
                if(!sendFrame(true, (unsigned char)(0x0), message.substr(bytesSent, bytesSent + bytesLeft))){
                    return false;
                }
                bytesSent += bytesLeft;
            }
        }
    }
    
    return true;
}

// Handle websocket message from client
void WebSocketConnection::loop(){
    //std::cout << "WEBSOCKET LOOP\n";
    const char *term=" \t\r\n";
    
    // Frame info
    bool fin = false;
    bool mask = false;
    int maskData = 0;
    unsigned int opcode = 0;
    unsigned int payloadLength = 0;
    bool binary = false;
    
    // Current message
    std::string message = "";
    
    // Buffer to store HTTP messages
    std::vector<std::string> buffer;
    
    // Connection open
    onOpen();
    
    // Loop until connection dies
    while(!_dead){
        if(!_handshake){
            // Read characters from HTTP handshake request
            char c;
            if(skt_recvN(_socket,&c,1) != 0){
                fail(); // Connection failure
            }
            else if(strchr(term,c)){
	            if(c=='\r') continue; // Will be CR/LF; wait for LF
	            else if(message.compare("")){
	                buffer.push_back(message);
	                message = "";
	            }
	            else{
	                _handshake = parseHandshake(buffer);
	            }
            }
            else{
                message+=c; // normal character
            }
        }
        else{
            // Read first byte
            unsigned char byte;
            if(skt_recvN(_socket, &byte, sizeof(byte)) != 0){
                fail(); // Connection failure
            }
            
            // Check if connection died while reading
            if(_dead) break;

            // Sort out frame info from bits
            fin = (byte >> 7);
            opcode = byte & 0xF; // We skip the three RSV bits
            
            if(skt_recvN(_socket, &byte, 1) != 0){
                fail(); // Connection failure
            }
            
            // Check if connection died while reading
            if(_dead) break;
            
            // Sort out frame info from bits
            mask = byte >> 7;
            payloadLength = byte & 0x7F;
            
            // Read extended payload length if necessary
            if(payloadLength > 125){
                if(payloadLength == 126){
                    // Have to read bytes individually to fix byte endianness
                    unsigned char byte;
                    if(skt_recvN(_socket, &byte, 1) != 0){
                        fail(); // Connection failure
                    }
                    
                    // Check if connection died while reading
                    if(_dead) break;
                    
                    payloadLength = ((int)byte << 8);
                    
                    if(skt_recvN(_socket, &byte, 1) != 0){
                        fail(); // Connection failure
                    }
                    
                    // Check if connection died while reading
                    if(_dead) break;
                    
                    payloadLength += byte;
                    
                }
                else{
                    // Supported by standard, but too big for memory
                    std::cout << "Unsupported payload length\n";
                }
            }
            
            // Read mask for masked data
            if(mask){
                if(skt_recvN(_socket, &maskData, sizeof(maskData)) != 0){
                    fail(); // Connection failure
                }
                
                // Check if connection died while reading
                if(_dead) break;
            }
            
            // Read payload
            char recieved[payloadLength];
            if(skt_recvN(_socket, &recieved, payloadLength) != 0){
                fail(); // Connection failure
            }
            
            // Check if connection died while reading
            if(_dead) break;
            
            // Apply mask for masked data
            if(mask){
                // Treat maskData int as character array for masking
                char * maskArray = (char *)(&maskData);
                for(std::size_t i=0; i<payloadLength; i++){
                    recieved[i] = recieved[i] ^ maskArray[i%4];
                }
            }
            
            // Append recieved data to current message
            message.append(std::string(recieved, payloadLength));
            
            // Respond to frame based upon fin bit and opcode
            if(fin){
                if(opcode == 0x1 || (opcode == 0x0 && binary == false)){
                    // Text message
                    onMessage(message);
                    sendMessage(message);
                }
                else if(opcode == 0x2 || (opcode == 0x0 && binary == true)){
                    // Binary message
                    onMessage(message);
                    sendMessage(message);
                }
                else if(opcode == 0x8){
                    fail();
                }
                else if(opcode == 0x9){
                    std::cout << "Ping recieved\n";
                    // Send pong
                }
                else if(opcode == 0xA){
                    std::cout << "Pong recieved\n";
                }
                else{
                    // Some non message opcode for finished message
                    std::cout << "Invalid opcode for fin=1\n";
                }
                message = "";
            }
            else{
                // Message not finished
                if(opcode == 1){
                    // Text message
                    binary = false;
                }
                else if(opcode == 2){
                    // Binary message
                    binary = true;
                }
                else if(opcode == 0x8){
                    fail();
                }
                else if(opcode == 0x9){
                    std::cout << "Ping recieved\n";
                    // Send pong
                }
                else if(opcode == 0xA){
                    std::cout << "Pong recieved\n";
                }
                else if(opcode != 0){
                    // Some non continue opcode for unfinished message
                    std::cout << "Invalid opcode for fin=0\n";
                }
            }
        }
    }
}

// Parse handshake and respond if correct
bool WebSocketConnection::parseHandshake(const std::vector<std::string> & buffer){
    // Create error HTTP response
    std::string error("HTTP/1.1 404 Not Found\nConetent-type: text/html\nContent-length: 0");
    error.append("\n\n");
    
    // Check HTTP header is correct
    if(buffer[0].compare("GET") || buffer[1].compare("/") || buffer[2].compare("HTTP/1.1")){
        sendTCP(error);
        return false;
    }
    
    // Read request and organize by attribute name
    std::map<std::string, std::vector<std::string>> attributes;
    std::string header = "";
    for(std::size_t i=3; i<buffer.size(); i++){
        if(buffer[i].back() == ':'){
            // Remove ':' from end and store key
            header = buffer[i].substr(0,buffer[i].size()-1);
        }
        else if(header.compare("")){
            attributes[header].push_back(buffer[i]);
        }
    }
    
    // Check if client sent a key
    if(attributes.count("Sec-WebSocket-Key") < 1){
        sendTCP(error);
        return false;
    }
    
    // Compute server accept key from client key
    std::string key = base64Encode(sha1(attributes["Sec-WebSocket-Key"].front() + _magicString));
    
    // Make sure client attributes are correct
    if(attributes["Sec-WebSocket-Version"].front().compare("13") ||
       attributes["Upgrade"].front().compare("websocket") ||
       attributes["Connection"].front().compare("Upgrade")){
        sendTCP(error);
        return false;
    }
    
    // Generate server response
    std::string response("HTTP/1.1 101 Switching Protocols\nUpgrade: websocket\nConnection: Upgrade\n");
    response.append("Connection: Upgrade\nSec-WebSocket-Accept: ");
    response.append(key);
    response.append("\n\n");
    sendTCP(response);
    
    // Connection open
    onOpen();
    
    return true;
}

// Send a websocket frame with the given parameters
// Must be called with payload.size() <= USHRT_MAX
bool WebSocketConnection::sendFrame(bool fin, unsigned char opcode, const std::string & payload){
    std::string frame = "";
    char byte;
    
    byte = fin ? (0x80) : (0x00);   // Set fin bit
    byte = byte | opcode;   // Set opcode as provided
    frame.push_back(byte);
    
    if(payload.size() < 125){
        byte = (char)payload.size(); //
        byte = byte & 0x7F; // Make sure mask bit is not set
        frame.push_back(byte);
    }
    else{
        // Sort out frame info from bits
        byte = 0x7E;
        frame.push_back(byte);
        
        // Append second byte of payload length
        byte = (char)(payload.size() >> 8);
        frame.push_back(byte);
        
        // Append first byte of payload length
        byte = (char)payload.size();
        frame.push_back(byte);
    }
    
    frame.append(payload);
    
    return sendTCP(frame);
}

// Send error message to client
bool WebSocketConnection::sendTCP(const std::string & message){
    // Don't add +1 to size, we don't want to send the c string end character
    if(skt_sendN(_socket, message.c_str(), message.size()) != 0){
        fail();
        return false;
    }
    return true;
}
