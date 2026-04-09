#include <zmq.hpp>
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
using namespace std;

int main() {
    zmq::context_t context(1);

    zmq::socket_t receiver(context, ZMQ_PULL);
    receiver.connect("tcp://10.43.100.176:5558");
    receiver.set(zmq::sockopt::rcvtimeo, 10000);

    zmq::socket_t heartbeat(context, ZMQ_PUB);
    heartbeat.bind("tcp://*:5561");

    ofstream archivo("database.json", ios::app);
    cout << "BD principal conectada a PC2, esperando datos..." << endl;

    while (true) {
        string ping = "ALIVE";
        zmq::message_t hb(ping.begin(), ping.end());
        heartbeat.send(hb, zmq::send_flags::none);

        zmq::message_t msg;
        auto result = receiver.recv(msg, zmq::recv_flags::none);
        if (result) {
            string data(static_cast<char*>(msg.data()), msg.size());
            archivo << data << "\n";
            archivo.flush();
            cout << "DB actualizada: " << data << endl;
        } else {
            cout << "[ALERTA] Sin datos de PC2..." << endl;
        }
    }
}