#include <iostream>
#include <stdio.h>
#include <zmq.hpp>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <thread>

using namespace std;

// los sensores deben tener el nombre antes de  es decir sensor ESP-id

int main()
{
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_PUB);

    socket.connect("tcp://localhost:5555");

    srand(time(NULL));

    while (true)
    {
        int volumen = rand() % 20;
        int velocidad = rand() % 50;
        int sensorID = rand() % 100;
        int interseccion = rand() % 20;
        int vehiculosContados = rand() % 15;
        int intervaloSegundos = 30;

        string evento = "{ \"sensor  \":\"espira\", \"sensorID \": \"ESP-" + to_string(sensorID) + "\", \"interseccion\":" + to_string(interseccion) + ", \"vehiculosContados\":" + to_string(vehiculosContados) + ", \"intervaloSegundos\":" + to_string(intervaloSegundos) + ", \"timestamp_inicio\": " + to_string(chrono::system_clock::now().time_since_epoch().count()) + ", \"timestamp_fin\": " + to_string(chrono::system_clock::now().time_since_epoch().count() + intervaloSegundos * 1000000000) + " }";

        zmq::message_t msg(evento.begin(), evento.end());
        socket.send(msg, zmq::send_flags::none);
        cout << "Evento detectado y enviado" << evento << endl;

        std::this_thread::sleep_for(std::chrono::seconds(5));
    }
}