#include <zmq.hpp>
#include <iostream>
#include <string>
#include <map>
using namespace std;

struct Semaforo {
    string estado = "ROJO";
    int duracion  = 15;
};

map<string, Semaforo> semaforos;

string extraerValorString(const string& json, const string& clave) {
    string buscador = "\"" + clave + "\": \"";
    size_t pos = json.find(buscador);
    if (pos == string::npos) {
        buscador = "\"" + clave + "\":\"";
        pos = json.find(buscador);
    }
    if (pos == string::npos) return "";
    pos += buscador.size();
    size_t fin = json.find("\"", pos);
    return json.substr(pos, fin - pos);
}

int extraerValorInt(const string& json, const string& clave) {
    string buscador = "\"" + clave + "\": ";
    size_t pos = json.find(buscador);
    if (pos == string::npos) {
        buscador = "\"" + clave + "\":";
        pos = json.find(buscador);
    }
    if (pos == string::npos) return 15;
    pos += buscador.size();
    while (pos < json.size() && json[pos] == ' ') pos++;
    size_t fin = pos;
    while (fin < json.size() && isdigit(json[fin])) fin++;
    if (fin == pos) return 15;
    return stoi(json.substr(pos, fin - pos));
}

int main() {
    zmq::context_t context(1);

    zmq::socket_t receiver(context, ZMQ_PULL);
    receiver.connect("tcp://localhost:5557");

    cout << "Servicio de semáforos encendido..." << endl;

    while (true) {
        zmq::message_t msg;
        receiver.recv(msg, zmq::recv_flags::none);
        string cmd(static_cast<char*>(msg.data()), msg.size());

        string interseccion = extraerValorString(cmd, "interseccion");
        string estado       = extraerValorString(cmd, "estado");
        string razon        = extraerValorString(cmd, "razon");
        int duracion        = extraerValorInt(cmd, "duracion_seg");

        if (interseccion.empty()) {
            cout << "[SEMAFORO] Comando inválido: " << cmd << endl;
            continue;
        }

        Semaforo& sem = semaforos[interseccion];
        string estadoAnterior = sem.estado;
        sem.estado   = estado;
        sem.duracion = duracion;

        cout << "[SEMAFORO] " << interseccion
             << " | " << estadoAnterior << " → " << estado
             << " | duración: " << duracion << "s"
             << " | razón: " << razon << endl;
    }
}
