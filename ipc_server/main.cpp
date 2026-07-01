#include "../common/zmq_helpers.h"

#include <QCoreApplication>
#include <QByteArray>
#include <QDateTime>
#include <QDebug>
#include <QString>
#include <QStringList>

#include <cerrno>
#include <csignal>

#include <zmq.h>

static volatile std::sig_atomic_t g_running = 1;

static void handleSignal(int)
{
    g_running = 0;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("ipc_server"));

    std::signal(SIGINT, handleSignal);
    std::signal(SIGTERM, handleSignal);

    const QString endpoint = endpointFromArguments(app.arguments());
    warnIfIpcTransportUnsupported(endpoint);

    void *context = zmq_ctx_new();
    if (!context) {
        qCritical().noquote() << "zmq_ctx_new failed:" << zmqErrorString();
        return 1;
    }

    void *socket = zmq_socket(context, ZMQ_REP);
    if (!socket) {
        qCritical().noquote() << "zmq_socket failed:" << zmqErrorString();
        zmq_ctx_term(context);
        return 1;
    }

    const int lingerMs = 0;
    zmq_setsockopt(socket, ZMQ_LINGER, &lingerMs, sizeof(lingerMs));

    const QByteArray endpointBytes = endpoint.toUtf8();
    if (zmq_bind(socket, endpointBytes.constData()) != 0) {
        qCritical().noquote() << "zmq_bind failed:" << endpoint << zmqErrorString();
        zmq_close(socket);
        zmq_ctx_term(context);
        return 1;
    }

    qInfo().noquote() << "ZeroMQ REP server started at" << endpoint;
    qInfo().noquote() << "Press Ctrl+C to stop.";

    while (g_running) {
        zmq_pollitem_t items[] = {
            {socket, 0, ZMQ_POLLIN, 0}
        };

        const int pollResult = zmq_poll(items, 1, 100);
        if (pollResult < 0) {
            if (zmq_errno() == EINTR) {
                continue;
            }
            qCritical().noquote() << "zmq_poll failed:" << zmqErrorString();
            break;
        }

        if ((items[0].revents & ZMQ_POLLIN) == 0) {
            QCoreApplication::processEvents();
            continue;
        }

        zmq_msg_t request;
        zmq_msg_init(&request);
        const int received = zmq_msg_recv(&request, socket, 0);
        if (received < 0) {
            qWarning().noquote() << "zmq_msg_recv failed:" << zmqErrorString();
            zmq_msg_close(&request);
            continue;
        }

        const QByteArray payload(static_cast<const char *>(zmq_msg_data(&request)),
                                 static_cast<int>(zmq_msg_size(&request)));
        zmq_msg_close(&request);

        const QString requestText = QString::fromUtf8(payload);
        qInfo().noquote() << "recv:" << requestText;

        const QString replyText = QStringLiteral("server reply [%1]: %2")
                                      .arg(QDateTime::currentDateTime().toString(Qt::ISODate),
                                           requestText);
        const QByteArray replyBytes = replyText.toUtf8();
        if (zmq_send(socket, replyBytes.constData(), replyBytes.size(), 0) < 0) {
            qWarning().noquote() << "zmq_send failed:" << zmqErrorString();
        }
    }

    qInfo().noquote() << "Stopping server.";
    zmq_close(socket);
    zmq_ctx_shutdown(context);
    zmq_ctx_term(context);
    return 0;
}
