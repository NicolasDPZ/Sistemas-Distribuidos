#include <zmq.hpp>
#include <iostream>
#include <fstream>

using namespace std;

int main() {

    zmq::context_t context(1);
    zmq::socket_t receiver(context, ZMQ_SUB);
    receiver.connect("tcp://localhost:5556");   //10.43.100.176
    receiver.set(zmq::sockopt::subscribe, "");
    ofstream archivo("database.txt", ios::app);


    while(true){

        zmq::message_t msg;
        receiver.recv(msg, zmq::recv_flags::none);

        string data(static_cast<char*>(msg.data()), msg.size());

        archivo << data << endl;

        cout << "DB actuializada " << data << endl;
    }
}
