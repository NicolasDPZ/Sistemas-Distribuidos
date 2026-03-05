#include <zmq.hpp>
#include <iostream>

int main() {

    zmq::context_t context(1);

    zmq::socket_t frontend(context, ZMQ_SUB);
    frontend.bind("tcp://*:5555");

    frontend.set(zmq::sockopt::subscribe, "");

    zmq::socket_t backend(context, ZMQ_PUB);
    backend.bind("tcp://*:5556");

    while (true) {

        zmq::message_t message;

        auto result = frontend.recv(message, zmq::recv_flags::none);

        if (result) {
            backend.send(message, zmq::send_flags::none);
        }

    }

}