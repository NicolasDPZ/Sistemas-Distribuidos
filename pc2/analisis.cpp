#include <zmq.hpp>
#include <iostream>
#include <string>
using namespace std;

// sacar los valores para los sensores
string extraerValorString(const string &json, const string &clave)
{
    string buscador = "\"" + clave + "\": \"";
    size_t pos = json.find(buscador);
    if (pos == string::npos)
    {
        buscador = "\"" + clave + "\":\"";
        pos = json.find(buscador);
    }
    if (pos == string::npos)
        return "";
    pos += buscador.size();
    size_t fin = json.find("\"", pos);
    return json.substr(pos, fin - pos);
}

int extraerValorInt(const string &json, const string &clave)
{
    string buscador = "\"" + clave + "\": ";
    size_t pos = json.find(buscador);
    if (pos == string::npos)
    {
        buscador = "\"" + clave + "\":";
        pos = json.find(buscador);
    }
    if (pos == string::npos)
        return 0;
    pos += buscador.size();
    while (pos < json.size() && json[pos] == ' ')
        pos++;
    size_t fin = pos;
    while (fin < json.size() && (isdigit(json[fin]) || json[fin] == '-'))
        fin++;
    if (fin == pos)
        return 0;
    return stoi(json.substr(pos, fin - pos));
}

int main()
{

    // conexiones pcs

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

    cout << "Analítica encendida..." << endl;

    zmq::pollitem_t items[] = {
        {subSocket, 0, ZMQ_POLLIN, 0},
        {repMonitoreo, 0, ZMQ_POLLIN, 0}};

    while (true)
    {
        zmq::poll(items, 2, 100);

        // eventos de los senores en sub
        if (items[0].revents & ZMQ_POLLIN)
        {
            zmq::message_t msg;
            subSocket.recv(msg, zmq::recv_flags::none);
            string texto(static_cast<char *>(msg.data()), msg.size());
            cout << "\n[EVENTO] " << texto << endl;

            int intNum = extraerValorInt(texto, "interseccion");
            string interseccion = "INT-" + to_string(intNum);
            int volumen = extraerValorInt(texto, "volumen");
            int velocidad = extraerValorInt(texto, "velocidad");
            int vehiculos = extraerValorInt(texto, "vehiculosContados");

            string estadoTrafico;
            string estadoSemaforo;
            int duracion;

            if (volumen < 5 && velocidad > 35 && vehiculos < 20)
            {
                estadoTrafico = "NORMAL";
                estadoSemaforo = "VERDE";
                duracion = 15;
                cout << "[ANALITICA] " << interseccion << " → NORMAL. Ciclo estándar 15s." << endl;
            }
            else if (volumen > 10 || velocidad < 20)
            {
                estadoTrafico = "CONGESTION";
                estadoSemaforo = "ROJO";
                duracion = 45;
                cout << "[ANALITICA] " << interseccion << " → CONGESTIÓN. Rojo 45s." << endl;
            }
            else if (vehiculos > 30)
            {
                estadoTrafico = "ALTA_DENSIDAD";
                estadoSemaforo = "VERDE";
                duracion = 60;
                cout << "[ANALITICA] " << interseccion << " → ALTA DENSIDAD. Verde extendido 60s." << endl;
            }
            else
            {
                estadoTrafico = "MODERADO";
                estadoSemaforo = "VERDE";
                duracion = 30;
                cout << "[ANALITICA] " << interseccion << " → MODERADO. Verde 30s." << endl;
            }

            string cmd = "{\"comando\":\"CAMBIAR_LUZ\","
                         "\"interseccion\":\"" +
                         interseccion + "\","
                                        "\"estado\":\"" +
                         estadoSemaforo + "\","
                                          "\"duracion_seg\":" +
                         to_string(duracion) + ","
                                               "\"razon\":\"" +
                         estadoTrafico + "\"}";

            zmq::message_t m1(texto.begin(), texto.end());
            zmq::message_t m2(texto.begin(), texto.end());
            zmq::message_t cmdMsg(cmd.begin(), cmd.end());

            pushMain.send(m1, zmq::send_flags::none);
            pushReplica.send(m2, zmq::send_flags::none);
            pushSemaforos.send(cmdMsg, zmq::send_flags::none);

            cout << "[ANALITICA] Enviado a BD principal, réplica y semáforos." << endl;
        }

        // monitoreo con req-rep
        if (items[1].revents & ZMQ_POLLIN)
        {
            zmq::message_t req;
            repMonitoreo.recv(req, zmq::recv_flags::none);
            string consulta(static_cast<char *>(req.data()), req.size());
            cout << "\n[MONITOREO REQ] " << consulta << endl;

            string respuesta;
            string tipo = extraerValorString(consulta, "tipo");

            if (tipo == "ESTADO_INTERSECCION")
            {
                string inter = extraerValorString(consulta, "interseccion");
                respuesta = "{\"ok\":true,\"interseccion\":\"" + inter + "\",\"mensaje\":\"consulta recibida\"}";
            }
            else if (tipo == "ESTADO_CIUDAD")
            {
                respuesta = "{\"ok\":true,\"mensaje\":\"ciudad operando con 16 intersecciones\"}";
            }
            else if (tipo == "VER_REGLAS")
            {
                respuesta = "{"
                            "\"reglas\":{"
                            "\"NORMAL\":\"Q < 5 AND Vp > 35 AND D < 20 → Verde 15s\","
                            "\"CONGESTION\":\"Q > 10 OR Vp < 20 → Rojo 45s\","
                            "\"ALTA_DENSIDAD\":\"D > 30 → Verde extendido 60s\","
                            "\"MODERADO\":\"resto de casos → Verde 30s\""
                            "}"
                            "}";
            }
            else
            {
                respuesta = "{\"error\":\"comando desconocido\"}";
            }

            zmq::message_t rep(respuesta.begin(), respuesta.end());
            repMonitoreo.send(rep, zmq::send_flags::none);
            cout << "[MONITOREO REP] " << respuesta << endl;
        }
    }
}
