#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QVector>
#include <QList>
#include <QLineF>

class Point {
public:
    QPointF position; // Posición del punto
    QList<Point*> connections; // Lista de puntos conectados

    Point(QPointF pos) : position(pos) {}

    void connectTo(Point* other) {
        if (!connections.contains(other)) {
            connections.append(other);
            other->connections.append(this); // Conexión bidireccional
        }
    }
};

class MyWidget : public QWidget {
    Q_OBJECT

public:
    MyWidget(QWidget *parent = nullptr) : QWidget(parent) {
        setWindowTitle("Programa Representacion de Grafos");
        resize(800, 600);
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        QPainter painter(this);
        painter.setPen(Qt::black);

        // Dibuja los puntos
        for (Point* point : points) {
            painter.drawEllipse(point->position, 7, 7); // Dibuja un círculo de radio 5
        }

        // Dibuja líneas entre los puntos conectados
        painter.setPen(Qt::black); // Cambia el color para las líneas de conexión
        for (Point* point : points) {
            for (Point* connectedPoint : point->connections) {
                painter.drawLine(point->position, connectedPoint->position); // Dibuja una línea entre puntos conectados
            }
        }
    }

    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) {
            // Agrega la posición del clic a la lista de puntos
            Point* newPoint = new Point(event->pos());
            points.append(newPoint);
            update(); // Solicita una actualización de la ventana para redibujar
        } else if (event->button() == Qt::RightButton) {
            // Conectar el punto seleccionado al hacer clic derecho
            selectPoint(event->pos());
            connectPoints();
        }
    }

private slots:
    void connectPoints() {
        // Solo conectar si hay al menos dos puntos seleccionados
        if (selectedPoints.size() >= 2) {
            for (int i = 0; i < selectedPoints.size(); ++i) {
                for (int j = i + 1; j < selectedPoints.size(); ++j) {
                    selectedPoints[i]->connectTo(selectedPoints[j]);
                }
            }
            selectedPoints.clear(); // Limpiar la selección después de conectar
            update(); // Solicita una actualización de la ventana para redibujar
        }
    }

    void selectPoint(const QPoint &point) {
        const int selectionRadius = 14; // Radio de selección

        // Verifica si el clic está dentro del radio de algún punto
        for (Point* p : points) {
            if (QLineF(p->position, point).length() <= selectionRadius) {
                // Agrega o quita un punto de la lista de puntos seleccionados
                if (selectedPoints.contains(p)) {
                    selectedPoints.removeAll(p); // Quitar si ya está seleccionado
                } else {
                    selectedPoints.append(p); // Agregar si no está seleccionado
                }
                break; // Salir del bucle después de encontrar un punto
            }
        }
        update(); // Solicita una actualización de la ventana para redibujar
    }

protected:
    void mouseDoubleClickEvent(QMouseEvent *event) override {
        // Permitir seleccionar puntos al hacer doble clic
        selectPoint(event->pos());
    }

private:
    QList<Point*> points; // Almacena los puntos donde se hace clic
    QList<Point*> selectedPoints; // Almacena los puntos seleccionados
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    MyWidget window;
    window.show();

    return app.exec();
}

#include "main.moc"
