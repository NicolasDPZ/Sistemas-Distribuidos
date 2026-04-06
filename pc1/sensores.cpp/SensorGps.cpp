#include <iostream>
#include <zmq.hpp>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <thread>
using namespace std;

string obtenerTimestamp() {
    auto now = chrono::system_clock::now();
    time_t t = chrono::system_clock::to_time_t(now);
    tm* gmt = gmtime(&t);
    stringstream ss;
    ss << put_time(gmt, "%Y-%m-%dT%H:%M:%SZ");
    return ss.str();
}

int main() {
    zmq::context_t context(1);
    zmq::socket_t socket(context, ZMQ_PUB);
    socket.connect("tcp://localhost:5555");

    srand(time(NULL));

    string filas[] = {"A","B","C","D"};
    int columnas   = 4;
    int intervalo  = 5;

    while (true) {
        for (int f = 0; f < 4; f++) {
            for (int c = 1; c <= columnas; c++) {
                string sensorID     = "GPS-" + filas[f] + to_string(c);
                string interseccion = "INT-" + filas[f] + to_string(c);
                int velocidad       = rand() % 80;
                int densidad        = rand() % 50;

                string nivel;
                if (velocidad < 10)       nivel = "ALTA";
                else if (velocidad <= 39) nivel = "NORMAL";
                else                      nivel = "BAJA";

                string evento = "{"
                    "\"sensor_id\":\"" + sensorID + "\","
                    "\"tipo_sensor\":\"gps\","
                    "\"interseccion\":\"" + interseccion + "\","
                    "\"nivel_congestion\":\"" + nivel + "\","
                    "\"velocidad\":" + to_string(velocidad) + ","
                    "\"densidad\":" + to_string(densidad) + ","
                    "\"timestamp\":\"" + obtenerTimestamp() + "\""
                    "}";

                zmq::message_t msg(evento.begin(), evento.end());
                socket.send(msg, zmq::send_flags::none);
                cout << "Enviado: " << evento << endl;
            }
        }
        this_thread::sleep_for(chrono::seconds(intervalo));
    }
}