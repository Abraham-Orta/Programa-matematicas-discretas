#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QVector>
#include <QList>
#include <QLineF>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStack>

// Clase que representa un punto en el grafo
class Punto {
public:
    QPointF posicion; // Posición del punto en el espacio 2D
    QList<Punto*> conexiones; // Lista de punteros a otros puntos conectados

    // Constructor que inicializa la posición del punto
    Punto(QPointF pos) : posicion(pos) {}

    // Método para conectar este punto con otro
    void conectarCon(Punto* otro) {
        // Verifica si la conexión ya existe
        if (!conexiones.contains(otro)) {
            conexiones.append(otro); // Agrega el otro punto a las conexiones
            otro->conexiones.append(this); // Conexión bidireccional
        }
    }
};

// Clase principal que representa el widget donde se dibuja el grafo
class MiWidget : public QWidget {
    Q_OBJECT // Macro necesaria para el uso de señales y slots en Qt

public:
    // Constructor del widget
    MiWidget(QWidget *padre = nullptr) : QWidget(padre) {
        setWindowTitle("Programa Representación de Grafos"); // Título de la ventana
        resize(800, 600); // Tamaño inicial de la ventana

        // Crear botones para deshacer y borrar
        QPushButton *botonDeshacer = new QPushButton("Deshacer", this);
        QPushButton *botonBorrar = new QPushButton("Borrar Todo", this);

        // Establecer un tamaño fijo para los botones
        botonDeshacer->setFixedSize(80, 30); // Ancho 80, Alto 30
        botonBorrar->setFixedSize(80, 30); // Ancho 80, Alto 30

        // Conectar señales de los botones a los slots correspondientes
        connect(botonDeshacer, &QPushButton::clicked, this, &MiWidget::deshacer);
        connect(botonBorrar, &QPushButton::clicked, this, &MiWidget::borrar);

        // Layout horizontal para los botones
        QHBoxLayout *layoutBotones = new QHBoxLayout();
        layoutBotones->addWidget(botonDeshacer); // Agregar botón de deshacer
        layoutBotones->addWidget(botonBorrar); // Agregar botón de borrar

        // Layout principal vertical
        QVBoxLayout *layoutPrincipal = new QVBoxLayout(this);
        layoutPrincipal->addLayout(layoutBotones); // Agregar el layout de botones al layout principal

        // Espaciador para empujar el área de dibujo hacia abajo
        layoutPrincipal->addSpacerItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

        setLayout(layoutPrincipal); // Establecer el layout principal
    }

protected:
    // Método que se llama para dibujar el widget
    void paintEvent(QPaintEvent *evento) override {
        QPainter pintor(this); // Crear un objeto QPainter para dibujar
        pintor.setPen(Qt::black); // Establecer el color del lápiz a negro

        // Dibuja los puntos
        for (Punto* punto : puntos) {
            if (puntosSeleccionados.contains(punto)) {
                pintor.setBrush(Qt::red); // Color para puntos seleccionados
            } else {
                pintor.setBrush(Qt::black); // Color para puntos no seleccionados
            }
            pintor.drawEllipse(punto->posicion, 7, 7); // Dibuja un círculo de radio 7
        }

        // Dibuja líneas entre los puntos conectados
        pintor.setPen(Qt::black); // Cambia el color para las líneas de conexión
        for (Punto* punto : puntos) {
            for (Punto* puntoConectado : punto->conexiones) {
                pintor.drawLine(punto->posicion, puntoConectado->posicion); // Dibuja una línea entre puntos conectados
            }
        }
    }

    // Método que se llama cuando se presiona un botón del mouse
    void mousePressEvent(QMouseEvent *evento) override {
        if (evento->button() == Qt::LeftButton) {
            // Agrega la posición del clic a la lista de puntos
            Punto* nuevoPunto = new Punto(evento->pos()); // Crear un nuevo punto en la posición del clic
            puntos.append(nuevoPunto); // Agregar el nuevo punto a la lista de puntos
            acciones.push_back({TipoAccion::Agregar, nuevoPunto}); // Guardar la acción de agregar
            update(); // Solicita una actualización de la ventana para redibujar
        } else if (evento->button() == Qt::RightButton) {
            // Conectar el punto seleccionado al hacer clic derecho
            seleccionarPunto(evento->pos()); // Seleccionar el punto en la posición del clic
            conectarPuntos(); // Intentar conectar los puntos seleccionados
        }
    }

private slots:
    // Método para conectar puntos seleccionados
    void conectarPuntos() {
        // Solo conectar si hay al menos dos puntos seleccionados
        if (puntosSeleccionados.size() >= 2) {
            for (int i = 0; i < puntosSeleccionados.size(); ++i) {
                for (int j = i + 1; j < puntosSeleccionados.size(); ++j) {
                    // Conectar los puntos seleccionados
                    puntosSeleccionados[i]->conectarCon(puntosSeleccionados[j]);
                    // Guardar la acción de conexión
                    acciones.push_back({TipoAccion::Conectar, puntosSeleccionados[i], puntosSeleccionados[j]});
                }
            }
            puntosSeleccionados.clear(); // Limpiar la selección después de conectar
            update(); // Solicita una actualización de la ventana para redibujar
        }
    }

    // Método para deshacer la última acción
    void deshacer() {
        if (!acciones.isEmpty()) {
            Accion ultimaAccion = acciones.pop(); // Obtener la última acción

            if (ultimaAccion.tipo == TipoAccion::Agregar) {
                // Eliminar el último punto agregado
                puntos.removeOne(ultimaAccion.punto); // Remover el punto de la lista
                delete ultimaAccion.punto; // Liberar memoria del punto eliminado
            } else if (ultimaAccion.tipo == TipoAccion::Conectar) {
                // Deshacer la conexión
                ultimaAccion.punto->conexiones.removeOne(ultimaAccion.puntoConectado); // Remover la conexión
                ultimaAccion.puntoConectado->conexiones.removeOne(ultimaAccion.punto); // Remover la conexión en la otra dirección
            }
            update(); // Solicita una actualización de la ventana para redibujar
        }
    }

    // Método para borrar todos los puntos y conexiones
    void borrar() {
        // Limpiar todos los puntos y conexiones
        qDeleteAll(puntos); // Eliminar todos los puntos de la memoria
        puntos.clear(); // Limpiar la lista de puntos
        puntosSeleccionados.clear(); // Limpiar la lista de puntos seleccionados
        acciones.clear(); // Limpiar la pila de acciones
        update(); // Solicita una actualización de la ventana para redibujar
    }

    // Método para seleccionar un punto basado en la posición del clic
    void seleccionarPunto(const QPoint &punto) {
        const int radioSeleccion = 14; // Radio de selección para detectar clics en puntos

        // Verifica si el clic está dentro del radio de algún punto
        for (Punto* p : puntos) {
            if (QLineF(p->posicion, punto).length() <= radioSeleccion) {
                // Agrega o quita un punto de la lista de puntos seleccionados
                if (puntosSeleccionados.contains(p)) {
                    puntosSeleccionados.removeAll(p); // Quitar si ya está seleccionado
                } else {
                    puntosSeleccionados.append(p); // Agregar si no está seleccionado
                }
                break; // Salir del bucle después de encontrar un punto
            }
        }
        update(); // Solicita una actualización de la ventana para redibujar
    }

protected:
    // Método que se llama cuando se hace doble clic en el widget
    void mouseDoubleClickEvent(QMouseEvent *evento) override {
        // Permitir seleccionar puntos al hacer doble clic
        seleccionarPunto(evento->pos()); // Seleccionar el punto en la posición del doble clic
    }

private:
    // Enumeración para definir los tipos de acciones
    enum class TipoAccion {
        Agregar, // Acción de agregar un punto
        Conectar // Acción de conectar dos puntos
    };

    // Estructura para almacenar información sobre una acción
    struct Accion {
        TipoAccion tipo; // Tipo de acción (Agregar o Conectar)
        Punto* punto; // Puntero al punto involucrado en la acción
        Punto* puntoConectado = nullptr; // Puntero al punto conectado (solo se usa para conexiones)
    };

    QList<Punto*> puntos; // Almacena los puntos donde se hace clic
    QList<Punto*> puntosSeleccionados; // Almacena los puntos seleccionados
    QStack<Accion> acciones; // Pila para deshacer acciones

};

// Función principal de la aplicación
int main(int argc, char *argv[]) {
    QApplication app(argc, argv); // Inicializa la aplicación Qt

    MiWidget ventana; // Crea una instancia del widget principal
    ventana.show(); // Muestra la ventana

    return app.exec(); // Ejecuta el bucle de eventos de la aplicación
}

#include "main.moc" // Incluir el archivo de implementación de señales y slots

