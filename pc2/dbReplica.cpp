#include <zmq.hpp>
#include <iostream>
#include <fstream>
using namespace std;

int main() {
    zmq::context_t context(1);

    zmq::socket_t receiver(context, ZMQ_PULL);
    receiver.bind("tcp://*:5559");

    ofstream archivo("backup.txt", ios::app);
    cout << "BD réplica esperando datos..." << endl;

    while (true) {
        zmq::message_t msg;
        receiver.recv(msg, zmq::recv_flags::none);
        string data(static_cast<char*>(msg.data()), msg.size());
        archivo << data << "\n";
        archivo.flush();
        cout << "DB réplica actualizada: " << data << endl;
    }
}
