#include "ManagerAdaptor.h"

#include <QDBusObjectPath>
#include <QLocalSocket>

static const QString SYSTEMD1_SERVICE = "org.freedesktop.systemd1";
static const QString SYSTEMD1_MANAGER_PATH = "/org/freedesktop/systemd1";

static const QString STATE_IDEL = "idle";
static const QString STATE_UPGRADING = "upgrading";
static const QString STATE_FAILED = "failed";

ManagerAdaptor::ManagerAdaptor(int upgradeStdoutFd, const QDBusConnection &bus, QObject *parent)
    : QObject(parent)
    , m_bus(bus)
    , m_server(new QLocalServer(this))
    , m_systemdManager(new org::freedesktop::systemd1::Manager(
          SYSTEMD1_SERVICE, SYSTEMD1_MANAGER_PATH, bus, this))
{
    m_server->listen(upgradeStdoutFd);
    connect(m_server, &QLocalServer::newConnection, this, [this] {
        auto *socket = m_server->nextPendingConnection();
        connect(socket, &QLocalSocket::disconnected, socket, &QLocalSocket::deleteLater);
        connect(socket, &QLocalSocket::readyRead, this, [this, socket] {
            while (socket->canReadLine()) {
                auto line = socket->readLine();
                parseUpgradeStdoutLine(line);
            }
        });
    });

    auto reply = m_systemdManager->GetUnit("dum-upgrade.service");
    reply.waitForFinished();
    if (!reply.isValid()) {
        // todo: log
        throw std::runtime_error("GetUnit dum-upgrade.service failed");
    }

    auto unitPath = reply.value();
    m_dumUpgradeUnit = new org::freedesktop::systemd1::Unit(SYSTEMD1_SERVICE,
                                                            unitPath.path(),
                                                            QDBusConnection::systemBus());
    m_bus.connect(SYSTEMD1_SERVICE,
                  unitPath.path(),
                  "org.freedesktop.DBus.Properties",
                  "PropertiesChanged",
                  this,
                  SLOT(onDumUpgradeUnitPropertiesChanged(const QString &,
                                                         const QVariantMap &,
                                                         const QStringList &)));
}

ManagerAdaptor::~ManagerAdaptor() = default;

void ManagerAdaptor::checkUpgrade() { }

void ManagerAdaptor::upgrade(const QDBusMessage &message)
{
    auto activeState = m_dumUpgradeUnit->activeState();
    if (activeState == "active" || activeState == "activating" || activeState == "deactivating") {
        m_bus.send(message.createErrorReply(QDBusError::AccessDenied, "An upgrade is in progress"));
        return;
    }
    m_dumUpgradeUnit->Start("replace");
}

void ManagerAdaptor::onDumUpgradeUnitPropertiesChanged(const QString &interfaceName,
                                                       const QVariantMap &changedProperties,
                                                       const QStringList &invalidatedProperties)
{
    if (interfaceName == "org.freedesktop.systemd1.Unit") {
        auto activeState = changedProperties.value("ActiveState").toString();
        if (activeState == "active" || activeState == "activating"
            || activeState == "deactivating") {
            m_state = STATE_UPGRADING;
            emit stateChanged(m_state);
        } else if (activeState == "failed") {
            m_state = STATE_FAILED;
            emit stateChanged(m_state);
        } else {
            m_state = STATE_IDEL;
            emit stateChanged(m_state);
        }
    }
}

static const QByteArray PROGRESS_PREFIX = "progressRate:";

void ManagerAdaptor::parseUpgradeStdoutLine(const QByteArray &line)
{
    if (line.startsWith(PROGRESS_PREFIX)) {
        auto tmp = QByteArrayView(line.begin() + PROGRESS_PREFIX.size(), line.end()).trimmed();
        auto colonIdx = tmp.indexOf(':');
        QString percentStr = tmp.sliced(0, colonIdx).trimmed().toByteArray();
        QString stage = tmp.sliced(colonIdx + 1).trimmed().toByteArray();

        emit progress({ stage, percentStr.toFloat() });
    }
}
