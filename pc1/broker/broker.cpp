#include <iostream>
#include <stdio.h>
#include <zmq.hpp>

int main(){
    zmq:: context_t context (1);

    zmq::socket_t frontend (context, ZMQ_ROUTER);
    frontend.bind("tcp://*:5555");
    frontend.setsockopt(ZMQ_SUBSCRIBE, "", 0);

    zmq::socket_t backend(context, ZMQ_PUB);
    backend.bind("tcp://*:5556");

    std::cout << "Broker iniciado..." << std::endl;

    while(true) {

        zmq::message_t message;

        frontend.recv(message);

        backend.send(message);
    }
}