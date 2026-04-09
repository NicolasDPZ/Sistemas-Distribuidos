#include <zmq.hpp>
#include <iostream>
#include <string>
#include <map>
#include <chrono>
using namespace std;

struct EstadoInter {
    int volumen   = 0;
    int velocidad = 0;
    int vehiculos = 0;
    string semaforo = "ROJO";
    string trafico  = "DESCONOCIDO";
};

map<string, EstadoInter> estadoCiudad;

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
    if (pos == string::npos) return 0;
    pos += buscador.size();
    while (pos < json.size() && json[pos] == ' ') pos++;
    size_t fin = pos;
    while (fin < json.size() && (isdigit(json[fin]) || json[fin] == '-')) fin++;
    if (fin == pos) return 0;
    return stoi(json.substr(pos, fin - pos));
}

int main() {
    zmq::context_t context(1);

    zmq::socket_t subSocket(context, ZMQ_SUB);
    subSocket.connect("tcp://10.43.100.43:5556");
    subSocket.set(zmq::sockopt::subscribe, "");

    zmq::socket_t pushMain(context, ZMQ_PUSH);
    pushMain.bind("tcp://*:5558");

    zmq::socket_t pushReplica(context, ZMQ_PUSH);
    pushReplica.connect("tcp://localhost:5559");

    zmq::socket_t pushSemaforos(context, ZMQ_PUSH);
    pushSemaforos.bind("tcp://*:5557");

    zmq::socket_t repMonitoreo(context, ZMQ_REP);
    repMonitoreo.bind("tcp://*:5560");

    zmq::socket_t subHeartbeat(context, ZMQ_SUB);
    subHeartbeat.connect("tcp://10.43.99.111:5561");
    subHeartbeat.set(zmq::sockopt::subscribe, "");

    bool pc3Activo = true;
    auto ultimoHB = chrono::steady_clock::now();

    cout << "Analitica encendida..." << endl;

    zmq::pollitem_t items[] = {
        {subSocket,    0, ZMQ_POLLIN, 0},
        {repMonitoreo, 0, ZMQ_POLLIN, 0},
        {subHeartbeat, 0, ZMQ_POLLIN, 0}
    };

    while (true) {
        zmq::poll(items, 3, 100);

        // ── Heartbeat de PC3 ─────────────────────────────
        if (items[2].revents & ZMQ_POLLIN) {
            zmq::message_t hb;
            subHeartbeat.recv(hb, zmq::recv_flags::none);
            ultimoHB = chrono::steady_clock::now();
            if (!pc3Activo) {
                cout << "[RECUPERACION] PC3 volvio. Redirigiendo a BD principal." << endl;
                pc3Activo = true;
            }
        }

        // Verificar si PC3 lleva mas de 12s sin responder
        auto ahora = chrono::steady_clock::now();
        if (chrono::duration_cast<chrono::seconds>(ahora - ultimoHB).count() > 12) {
            if (pc3Activo) {
                cout << "[FALLA] PC3 caido. Usando BD replica en PC2." << endl;
                pc3Activo = false;
            }
        }

        // ── Eventos de sensores ──────────────────────────
        if (items[0].revents & ZMQ_POLLIN) {
            zmq::message_t msg;
            subSocket.recv(msg, zmq::recv_flags::none);
            string texto(static_cast<char*>(msg.data()), msg.size());
            cout << "\n[EVENTO] " << texto << endl;

            string interseccion = extraerValorString(texto, "interseccion");
            int volumen         = extraerValorInt(texto, "volumen");
            int velocidad       = extraerValorInt(texto, "velocidad");
            int vehiculos       = extraerValorInt(texto, "vehiculos_contados");

            string estadoTrafico;
            string estadoSemaforo;
            int duracion;

            if (volumen < 5 && velocidad > 35 && vehiculos < 20) {
                estadoTrafico  = "NORMAL";
                estadoSemaforo = "VERDE";
                duracion       = 15;
                cout << "[ANALITICA] " << interseccion << " -> NORMAL. Ciclo estandar 15s." << endl;

            } else if (volumen > 10 || velocidad < 20) {
                estadoTrafico  = "CONGESTION";
                estadoSemaforo = "ROJO";
                duracion       = 45;
                cout << "[ANALITICA] " << interseccion << " -> CONGESTION. Rojo 45s." << endl;

            } else if (vehiculos > 30) {
                estadoTrafico  = "ALTA_DENSIDAD";
                estadoSemaforo = "VERDE";
                duracion       = 60;
                cout << "[ANALITICA] " << interseccion << " -> ALTA DENSIDAD. Verde extendido 60s." << endl;

            } else {
                estadoTrafico  = "MODERADO";
                estadoSemaforo = "VERDE";
                duracion       = 30;
                cout << "[ANALITICA] " << interseccion << " -> MODERADO. Verde 30s." << endl;
            }

            estadoCiudad[interseccion] = {volumen, velocidad, vehiculos, estadoSemaforo, estadoTrafico};

            string cmd = "{\"comando\":\"CAMBIAR_LUZ\","
                         "\"interseccion\":\"" + interseccion + "\","
                         "\"estado\":\"" + estadoSemaforo + "\","
                         "\"duracion_seg\":" + to_string(duracion) + ","
                         "\"razon\":\"" + estadoTrafico + "\"}";

            zmq::message_t m1(texto.begin(), texto.end());
            zmq::message_t m2(texto.begin(), texto.end());
            zmq::message_t cmdMsg(cmd.begin(), cmd.end());

            if (pc3Activo) {
                pushMain.send(m1, zmq::send_flags::dontwait);
                cout << "[ANALITICA] Enviado a BD principal, replica y semaforos." << endl;
            } else {
                cout << "[ANALITICA] PC3 inactivo. Enviando solo a replica." << endl;
            }

            pushReplica.send(m2, zmq::send_flags::none);
            pushSemaforos.send(cmdMsg, zmq::send_flags::none);
        }

        // ── Monitoreo REQ/REP ────────────────────────────
        if (items[1].revents & ZMQ_POLLIN) {
            zmq::message_t req;
            repMonitoreo.recv(req, zmq::recv_flags::none);
            string consulta(static_cast<char*>(req.data()), req.size());
            cout << "\n[MONITOREO REQ] " << consulta << endl;

            string respuesta;
            string tipo = extraerValorString(consulta, "tipo");

            if (tipo == "ESTADO_INTERSECCION") {
                string inter = extraerValorString(consulta, "interseccion");
                auto it = estadoCiudad.find(inter);
                if (it != estadoCiudad.end()) {
                    auto& e = it->second;
                    respuesta = "{\"interseccion\":\"" + inter + "\","
                                "\"volumen\":" + to_string(e.volumen) + ","
                                "\"velocidad\":" + to_string(e.velocidad) + ","
                                "\"vehiculos\":" + to_string(e.vehiculos) + ","
                                "\"semaforo\":\"" + e.semaforo + "\","
                                "\"trafico\":\"" + e.trafico + "\"}";
                } else {
                    respuesta = "{\"error\":\"interseccion no encontrada, espere datos del sensor\"}";
                }

            } else if (tipo == "ESTADO_CIUDAD") {
                respuesta = "{\"ok\":true,\"mensaje\":\"ciudad operando con 16 intersecciones\"}";

            } else if (tipo == "VER_REGLAS") {
                respuesta = "{\"reglas\":{"
                            "\"NORMAL\":\"Q < 5 AND Vp > 35 AND D < 20 -> Verde 15s\","
                            "\"CONGESTION\":\"Q > 10 OR Vp < 20 -> Rojo 45s\","
                            "\"ALTA_DENSIDAD\":\"D > 30 -> Verde extendido 60s\","
                            "\"MODERADO\":\"resto de casos -> Verde 30s\""
                            "}}";

            } else {
                respuesta = "{\"error\":\"comando desconocido\"}";
            }

            zmq::message_t rep(respuesta.begin(), respuesta.end());
            repMonitoreo.send(rep, zmq::send_flags::none);
            cout << "[MONITOREO REP] " << respuesta << endl;
        }
    }
}