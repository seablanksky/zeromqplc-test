#include "zmq_helpers.h"

#include <QCoreApplication>
#include <QDebug>
#include <QtGlobal>
#include <QStringList>

#include <zmq.h>

QString defaultEndpoint()
{
#ifdef Q_OS_WIN
    return QStringLiteral("tcp://127.0.0.1:5555");
#else
    return QStringLiteral("ipc:///tmp/qt_zeromq_demo.ipc");
#endif
}

QString endpointFromArguments(const QStringList &arguments)
{
    if (arguments.size() > 1) {
        return arguments.at(1);
    }

    return defaultEndpoint();
}

QString zmqErrorString()
{
    return QString::fromLocal8Bit(zmq_strerror(zmq_errno()));
}

bool warnIfIpcTransportUnsupported(const QString &endpoint)
{
#ifdef Q_OS_WIN
    if (endpoint.startsWith(QStringLiteral("ipc://"), Qt::CaseInsensitive)) {
        qWarning().noquote()
            << "This is a Windows environment. ZeroMQ ipc:// transport depends on UNIX domain sockets,"
               " so this endpoint may fail. Use"
            << defaultEndpoint()
            << "for local Windows IPC, or run ipc:// on Linux/macOS.";
        return true;
    }
#else
    Q_UNUSED(endpoint)
#endif
    return false;
}
