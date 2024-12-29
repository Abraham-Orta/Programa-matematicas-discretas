#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QVector>

class MyWidget : public QWidget {
    Q_OBJECT

public:
    MyWidget(QWidget *parent = nullptr) : QWidget(parent) {
        setWindowTitle("Dibuja Puntos y Conéctalos");
        resize(800, 600);
    }

protected:
    void paintEvent(QPaintEvent *event) override {
        QPainter painter(this);
        painter.setPen(Qt::black);

        // Dibuja líneas entre los puntos
        for (int i = 0; i < points.size() - 1; ++i) {
            painter.drawLine(points[i], points[i + 1]);
        }

        // Dibuja los puntos
        for (const QPoint &point : points) {
            painter.drawEllipse(point, 5, 5); // Dibuja un círculo de radio 5
        }
    }

    void mousePressEvent(QMouseEvent *event) override {
        if (event->button() == Qt::LeftButton) {
            points.append(event->pos()); // Agrega la posición del clic a la lista
            update(); // Solicita una actualización de la ventana para redibujar
        }
    }

private:
    QVector<QPoint> points; // Almacena los puntos donde se hace clic
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    MyWidget window;
    window.show();

    return app.exec();
}

#include "main.moc"
