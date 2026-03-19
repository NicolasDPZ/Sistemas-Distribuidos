#include <iostream>
#include <stdio.h>
#include <zmq.hpp>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <thread>

using namespace std;

// los sensores deben tener el nombre antes de  es decir sensor CAM-id

int main()
{
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_PUB);

    socket.connect("tcp://localhost:5555");

    srand(time(NULL));

    while (true)
    {
        int sensorID ;
        string nivelCongestion;
        int VelocidadPromedio;

        string evento = "{ \"sensor  \":\"gps\", \"sensorID \": \"GPS-" + to_string(sensorID) + "\", \"nivelCongestion\":\"" + nivelCongestion + "\", \"VelocidadPromedio\":" + to_string(VelocidadPromedio) + ", \"timestamp\": " + to_string(chrono::system_clock::now().time_since_epoch().count()) + " }";

        zmq::message_t msg(evento.begin(), evento.end());
        socket.send(msg, zmq::send_flags::none);
        cout << "Evento detectado y enviado" << evento << endl;

        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}