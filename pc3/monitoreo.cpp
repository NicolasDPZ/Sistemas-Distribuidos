#include <zmq.hpp>
#include <iostream>
#include <string>
using namespace std;

int main() {
    zmq::context_t context(1);

    zmq::socket_t reqSocket(context, ZMQ_REQ);
    reqSocket.connect("tcp://10.43.100.176:5560");

    cout << "Servicio de monitoreo encendido..." << endl;
    cout << "Comandos disponibles:" << endl;
    cout << "  1 - Consultar estado de una interseccion" << endl;
    cout << "  2 - Consultar estado de toda la ciudad" << endl;
    cout << " 3 - Consultar las reglas del sistema" << endl;
    cout << "  0 - Salir" << endl;

    while (true) {
        cout << "\nOpcion: ";
        int opcion;
        cin >> opcion;

        if (opcion == 0) break;

        string mensaje;

        if (opcion == 1) {
            cout << "Interseccion (ej: INT-A1): ";
            string inter;
            cin >> inter;
            mensaje = "{\"tipo\":\"ESTADO_INTERSECCION\",\"interseccion\":\"" + inter + "\"}";

        } else if (opcion == 2) {
            mensaje = "{\"tipo\":\"ESTADO_CIUDAD\"}";


        } else if (opcion == 3) {
            mensaje = "{\"tipo\":\"REGLAS_INTERSECCIONES\"}";
                
        } else {
            cout << "Opcion invalida." << endl;
            continue;
        }

        zmq::message_t req(mensaje.begin(), mensaje.end());
        reqSocket.send(req, zmq::send_flags::none);
        cout << "Solicitud enviada, esperando respuesta..." << endl;

        zmq::message_t rep;
        reqSocket.recv(rep, zmq::recv_flags::none);
        string respuesta(static_cast<char*>(rep.data()), rep.size());
        cout << "Respuesta: " << respuesta << endl;
    }

    return 0;
}