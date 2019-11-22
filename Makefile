CC=g++ -std=c++17

all: client server

client: client.cpp rpcs.cpp
	${CC} $^ -o $@

server: server.cpp rpcs.cpp
	${CC} $^ -o $@

.phony clean:
	rm -f client server
