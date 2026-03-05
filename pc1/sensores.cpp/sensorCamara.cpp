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

    srand(time(0));

    while (true)
    {

        int volumen = rand() % 20;
        int velocidad = rand() % 50;
        int sensorID;
        int interseccion;

        string evento = "{ \"sensor  \":\"camara\", \"sensorID \"\": \"\"" + to_string(sensorID) + "\", \"interseccion\":" + to_string(interseccion) + ", \"volumen\":" + to_string(volumen) + ", \"velocidad\":" + to_string(velocidad) + ", \"timestamp\": " + to_string(chrono::system_clock::now().time_since_epoch().count()) + " }";

        zmq::message_t msg(evento.begin(), evento.end());
        socket.send(msg, zmq::send_flags::none);
        
        cout << "Evento detectado y enviado" << evento << endl;

        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    return 0;
}