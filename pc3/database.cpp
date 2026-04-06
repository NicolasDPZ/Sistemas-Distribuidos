#include <zmq.hpp>
#include <iostream>
#include <fstream>
using namespace std;

int main() {
    zmq::context_t context(1);

    zmq::socket_t receiver(context, ZMQ_PULL);
    receiver.connect("tcp://10.43.100.176:5558");

    ofstream archivo("database.json", ios::app);
    cout << "BD principal conectada a PC2, esperando datos..." << endl;

    while (true) {
        zmq::message_t msg;
        receiver.recv(msg, zmq::recv_flags::none);
        string data(static_cast<char*>(msg.data()), msg.size());
        archivo << data << "\n";
        archivo.flush();
        cout << "DB actualizada: " << data << endl;
    }
}
