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

string obtenerTimestampOffset(int segundos) {
    auto now = chrono::system_clock::now() + chrono::seconds(segundos);
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
    int intervalo  = 30;

    while (true) {
        for (int f = 0; f < 4; f++) {
            for (int c = 1; c <= columnas; c++) {
                string sensorID     = "ESP-" + filas[f] + to_string(c);
                string interseccion = "INT-" + filas[f] + to_string(c);
                int vehiculos       = rand() % 20;

                string evento = "{"
                    "\"sensor_id\":\"" + sensorID + "\","
                    "\"tipo_sensor\":\"espira_inductiva\","
                    "\"interseccion\":\"" + interseccion + "\","
                    "\"vehiculos_contados\":" + to_string(vehiculos) + ","
                    "\"intervalo_segundos\":" + to_string(intervalo) + ","
                    "\"timestamp_inicio\":\"" + obtenerTimestamp() + "\","
                    "\"timestamp_fin\":\"" + obtenerTimestampOffset(intervalo) + "\""
                    "}";

                zmq::message_t msg(evento.begin(), evento.end());
                socket.send(msg, zmq::send_flags::none);
                cout << "Enviado: " << evento << endl;
            }
        }
        this_thread::sleep_for(chrono::seconds(intervalo));
    }
}