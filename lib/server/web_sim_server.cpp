/*
 * web_sim_server.h
 * Author: Aven Bross, Max Hesser-Knoll
 * Date: 10/4/2015
 * 
 * Description:
 * Websocket server to facilitate communication between web sim and SLAM system
*/

#include "web_sim_server.h"

/*
 * class WebSimServer : WebSocketServer
 * Subclass of WebSocketServer that connects webGL simulation to backend localization
 */

// Make a new connection for the server
std::shared_ptr<TCPConnection> WebSimServer::makeConnection(int socket, const sockaddr & clientAddress){
    return std::make_shared<WebSimConnection>(socket, this, clientAddress);
}

/*
 * class WebSimConnection : WebSocketConnection
 * Subclass of WebSocketConnection that connects webGL simulation to backend localization
 */
 
// Constructor, does any initializations necessary then calls parent constructor
WebSimConnection::WebSimConnection(int socket, Server * server, const sockaddr & toAddress):
  WebSocketConnection(socket, server, toAddress) {}

void WebSimConnection::onMessage(const std::string & message){
	std::cout << "recieved: " << message << "\n";
    /*if(message[0]=='r'){
        _robotMap = RobotMap(10,20,pi()/12.0,16);
    }
    else{
        std::istringstream ss(message);
        std::string token;
        std::vector<PolarCoordinates> data;
        
        PolarCoordinates coords;
        int step = -1;
        
        while(std::getline(ss, token, ',')) {
            double num = 0;
            try{
                num = std::stod(token);
            }
            catch(std::exception e){
                return;
            }
            
            if(step == 1){
                coords._angle = num;
                data.push_back(coords);
                step = 0;
            }
            else{
                coords._r = num;
                step = 1;
            }
        }
        
        RobotMapPoint p = _robotMap.feasiblePose(data);
        
        for(const auto & point : data){
            _robotMap.addPoint(point);
        }
        
        std::string result(std::to_string(p.getX()));
        result += ",";
        result += std::to_string(p.getY());
        result += ",";
        result += std::to_string(p.getAngle());
        std::cout << "sending: " << result << "\n";
        sendMessage(result);
    }*/
}