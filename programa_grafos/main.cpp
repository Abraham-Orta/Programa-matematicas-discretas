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

class Punto {
public:
    QPointF posicion; // Posición del punto
    QList<Punto*> conexiones; // Lista de puntos conectados

    Punto(QPointF pos) : posicion(pos) {}

    void conectarCon(Punto* otro) {
        if (!conexiones.contains(otro)) {
            conexiones.append(otro);
            otro->conexiones.append(this); // Conexión bidireccional
        }
    }
};

class MiWidget : public QWidget {
    Q_OBJECT

public:
    MiWidget(QWidget *padre = nullptr) : QWidget(padre) {
        setWindowTitle("Programa Representación de Grafos");
        resize(800, 600);

        // Crear botones
        QPushButton *botonDeshacer = new QPushButton("Deshacer", this);
        QPushButton *botonBorrar = new QPushButton("Borrar Todo", this);

        // Establecer un tamaño fijo para los botones
        botonDeshacer->setFixedSize(80, 30); // Ancho 80, Alto 30
        botonBorrar->setFixedSize(80, 30); // Ancho 80, Alto 30

        // Conectar señales de los botones a los slots
        connect(botonDeshacer, &QPushButton::clicked, this, &MiWidget::deshacer);
        connect(botonBorrar, &QPushButton::clicked, this, &MiWidget::borrar);

        // Layout para los botones
        QHBoxLayout *layoutBotones = new QHBoxLayout();
        layoutBotones->addWidget(botonDeshacer);
        layoutBotones->addWidget(botonBorrar);

        // Layout principal
        QVBoxLayout *layoutPrincipal = new QVBoxLayout(this);
        layoutPrincipal->addLayout(layoutBotones); // Agregar el layout de botones al layout principal

        // Espaciador para empujar el área de dibujo hacia abajo
        layoutPrincipal->addSpacerItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

        setLayout(layoutPrincipal);
    }

protected:
    void paintEvent(QPaintEvent *evento) override {
        QPainter pintor(this);
        pintor.setPen(Qt::black);

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

    void mousePressEvent(QMouseEvent *evento) override {
        if (evento->button() == Qt::LeftButton) {
            // Agrega la posición del clic a la lista de puntos
            Punto* nuevoPunto = new Punto(evento->pos());
            puntos.append(nuevoPunto);
            acciones.push_back({TipoAccion::Agregar, nuevoPunto}); // Guardar acción
            update(); // Solicita una actualización de la ventana para redibujar
        } else if (evento->button() == Qt::RightButton) {
            // Conectar el punto seleccionado al hacer clic derecho
            seleccionarPunto(evento->pos());
            conectarPuntos();
        }
    }

private slots:
    void conectarPuntos() {
        // Solo conectar si hay al menos dos puntos seleccionados
        if (puntosSeleccionados.size() >= 2) {
            for (int i = 0; i < puntosSeleccionados.size(); ++i) {
                for (int j = i + 1; j < puntosSeleccionados.size(); ++j) {
                    puntosSeleccionados[i]->conectarCon(puntosSeleccionados[j]);
                    acciones.push_back({TipoAccion::Conectar, puntosSeleccionados[i], puntosSeleccionados[j]}); // Guardar acción
                }
            }
            puntosSeleccionados.clear(); // Limpiar la selección después de conectar
            update(); // Solicita una actualización de la ventana para redibujar
        }
    }

    void deshacer() {
        if (!acciones.isEmpty()) {
            Accion ultimaAccion = acciones.pop(); // Obtener la última acción

            if (ultimaAccion.tipo == TipoAccion::Agregar) {
                // Eliminar el último punto agregado
                puntos.removeOne(ultimaAccion.punto);
                delete ultimaAccion.punto; // Liberar memoria
            } else if (ultimaAccion.tipo == TipoAccion::Conectar) {
                // Deshacer la conexión
                ultimaAccion.punto->conexiones.removeOne(ultimaAccion.puntoConectado);
                ultimaAccion.puntoConectado->conexiones.removeOne(ultimaAccion.punto);
            }
            update(); // Solicita una actualización de la ventana para redibujar
        }
    }

    void borrar() {
        // Limpiar todos los puntos y conexiones
        qDeleteAll(puntos); // Eliminar todos los puntos
        puntos.clear();
        puntosSeleccionados.clear();
        acciones.clear(); // Limpiar la pila de acciones
        update(); // Solicita una actualización de la ventana para redibujar
    }

    void seleccionarPunto(const QPoint &punto) {
        const int radioSeleccion = 14; // Radio de selección

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
    void mouseDoubleClickEvent(QMouseEvent *evento) override {
        // Permitir seleccionar puntos al hacer doble clic
        seleccionarPunto(evento->pos());
    }

private:
    enum class TipoAccion {
        Agregar,
        Conectar
    };

    struct Accion {
        TipoAccion tipo;
        Punto* punto;
        Punto* puntoConectado = nullptr; // Solo se usa para conexiones
    };

    QList<Punto*> puntos; // Almacena los puntos donde se hace clic
    QList<Punto*> puntosSeleccionados; // Almacena los puntos seleccionados
    QStack<Accion> acciones; // Pila para deshacer acciones
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    MiWidget ventana;
    ventana.show();

    return app.exec();
}

#include "main.moc"
