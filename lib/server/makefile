# Specify compiler
COMP = g++ -lssl -lcrypto -std=c++1y -O2 -Wall -Wno-unused-result

# Specify target
all: web_sim_test

# Build executable
web_sim_test: socket.o server.o crypto.o websocket_server.o angle.o localization_point_normal.o web_sim_server.o web_sim_test.o
	$(COMP) socket.o server.o crypto.o websocket_server.o angle.o localization_point_normal.o web_sim_server.o web_sim_test.o -pthread -o web_sim_test

web_sim_test.o: web_sim_test.cpp
	$(COMP) -c web_sim_test.cpp

web_sim_server.o: web_sim_server.cpp
	$(COMP) -c web_sim_server.cpp

localization_point_normal.o: localization_point_normal.cpp
	$(COMP) -c localization_point_normal.cpp

angle.o: angle.cpp
	$(COMP) -c angle.cpp

websocket_server.o: networking/websocket_server.cpp
	$(COMP) -c networking/websocket_server.cpp
	
crypto.o: cryptography/crypto.cpp
	$(COMP) -c cryptography/crypto.cpp

server.o: networking/server.cpp
	$(COMP) -c networking/server.cpp
	
socket.o: networking/osl/socket.cpp
	$(COMP) -c networking/osl/socket.cpp

# Clean build
clean:
	rm *.o web_sim_test

