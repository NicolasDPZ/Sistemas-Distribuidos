#include <iostream>
#include <stdio.h>
#include <zmq.hpp>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <thread>

using namespace std;


string obtenerTimestampISO()
{
    auto now = chrono::system_clock::now();
    time_t tiempo = chrono::system_clock::to_time_t(now);

    tm *gmt = gmtime(&tiempo);

    stringstream ss;
    ss << put_time(gmt, "%Y-%m-%dT%H:%M:%SZ");

    return ss.str();
}


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
        int sensorID;
        int interseccion;

        string evento = "{ \"sensor\":\"camara\", \"sensorID\": \"CAM-" + to_string(sensorID) + "\", \"interseccion\":" + to_string(interseccion) + ", \"volumen\":" + to_string(volumen) + ", \"velocidad\":" + to_string(velocidad) + ", \"timestamp\": " + obtenerTimestampISO() + " }";

        zmq::message_t msg(evento.begin(), evento.end());
        socket.send(msg, zmq::send_flags::none);
        
        cout << "Evento detectado y enviado" << evento << endl;

        std::this_thread::sleep_for(std::chrono::seconds(5));
    }

    return 0;
}
