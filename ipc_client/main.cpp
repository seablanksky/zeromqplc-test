#include "../common/zmq_helpers.h"

#include <QCoreApplication>
#include <QByteArray>
#include <QDateTime>
#include <QDebug>
#include <QThread>
#include <QString>
#include <QStringList>

#include <zmq.h>

static int messageCountFromArguments(const QStringList &arguments)
{
    if (arguments.size() <= 2) {
        return 5;
    }

    bool ok = false;
    const int value = arguments.at(2).toInt(&ok);
    return ok && value > 0 ? value : 5;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName(QStringLiteral("ipc_client"));

    const QStringList arguments = app.arguments();
    const QString endpoint = endpointFromArguments(arguments);
    const int messageCount = messageCountFromArguments(arguments);
    warnIfIpcTransportUnsupported(endpoint);

    void *context = zmq_ctx_new();
    if (!context) {
        qCritical().noquote() << "zmq_ctx_new failed:" << zmqErrorString();
        return 1;
    }

    void *socket = zmq_socket(context, ZMQ_REQ);
    if (!socket) {
        qCritical().noquote() << "zmq_socket failed:" << zmqErrorString();
        zmq_ctx_term(context);
        return 1;
    }

    const int lingerMs = 0;
    zmq_setsockopt(socket, ZMQ_LINGER, &lingerMs, sizeof(lingerMs));

    const QByteArray endpointBytes = endpoint.toUtf8();
    if (zmq_connect(socket, endpointBytes.constData()) != 0) {
        qCritical().noquote() << "zmq_connect failed:" << endpoint << zmqErrorString();
        zmq_close(socket);
        zmq_ctx_term(context);
        return 1;
    }

    qInfo().noquote() << "ZeroMQ REQ client connected to" << endpoint;

    for (int i = 1; i <= messageCount; ++i) {
        const QString requestText = QStringLiteral("hello from Qt client #%1 at %2")
                                        .arg(i)
                                        .arg(QDateTime::currentDateTime().toString(Qt::ISODate));
        const QByteArray requestBytes = requestText.toUtf8();

        if (zmq_send(socket, requestBytes.constData(), requestBytes.size(), 0) < 0) {
            qCritical().noquote() << "zmq_send failed:" << zmqErrorString();
            break;
        }
        qInfo().noquote() << "send:" << requestText;

        zmq_pollitem_t items[] = {
            {socket, 0, ZMQ_POLLIN, 0}
        };

        const int pollResult = zmq_poll(items, 1, 5000);
        if (pollResult < 0) {
            qCritical().noquote() << "zmq_poll failed:" << zmqErrorString();
            break;
        }

        if ((items[0].revents & ZMQ_POLLIN) == 0) {
            qCritical().noquote() << "timeout waiting for server reply";
            break;
        }

        zmq_msg_t reply;
        zmq_msg_init(&reply);
        const int received = zmq_msg_recv(&reply, socket, 0);
        if (received < 0) {
            qCritical().noquote() << "zmq_msg_recv failed:" << zmqErrorString();
            zmq_msg_close(&reply);
            break;
        }

        const QByteArray payload(static_cast<const char *>(zmq_msg_data(&reply)),
                                 static_cast<int>(zmq_msg_size(&reply)));
        zmq_msg_close(&reply);

        qInfo().noquote() << "recv:" << QString::fromUtf8(payload);
        QThread::sleep(1);
    }

    zmq_close(socket);
    zmq_ctx_shutdown(context);
    zmq_ctx_term(context);
    return 0;
}
