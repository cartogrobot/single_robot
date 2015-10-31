/*
 * web_sim_server.h
 * Author: Aven Bross, Max Hesser-Knoll
 * Date: 10/4/2015
 * 
 * Description:
 * Websocket server to facilitate communication between web sim and SLAM system
*/

#ifndef __WEB_SIM_SERVER_H
#define __WEB_SIM_SERVER_H

#include "networking/websocket_server.h"
#include "localization_point_normal.h"
#include <sstream>
#include <stdexcept>

// Subclass of TCPServer that recieves and handles websocket connections
class WebSimServer : public WebSocketServer{
public:
    using WebSocketServer::WebSocketServer;
    
protected:
    // Make a new UDP connection for the server
    virtual std::shared_ptr<TCPConnection> makeConnection(int socket, const sockaddr & clientAddress);
};

// Connection class representing a WebSocket connection to remote host
class WebSimConnection : public WebSocketConnection {
public:
    WebSimConnection(int socket, Server * server, const sockaddr & toAddress);
    
protected:
    virtual void onMessage(const std::string & message);
    
    // Stores map coordinates for this session
    Angle _robotMap;
};

#endif