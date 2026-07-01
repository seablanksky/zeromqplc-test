#ifndef ZMQ_HELPERS_H
#define ZMQ_HELPERS_H

#include <QString>

QString defaultEndpoint();
QString endpointFromArguments(const QStringList &arguments);
QString zmqErrorString();
bool warnIfIpcTransportUnsupported(const QString &endpoint);

#endif // ZMQ_HELPERS_H
