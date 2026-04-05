#include <zmq.hpp>
#include <iostream>
#include <string>

using namespace std;

int main(){

    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_SUB);

    socket.connect("tcp://localhost:5556");

    socket.set(zmq::sockopt::subscribe, "");
    cout <<"analitica prendido";

    while(true){

        zmq::message_t msg;

        auto result = socket.recv(msg, zmq::recv_flags::none);

        if(result){
            std::string texto(static_cast<char*>(msg.data()), msg.size());
            cout << "Dato recibido: " << texto << std::endl;
        }
    }

}

