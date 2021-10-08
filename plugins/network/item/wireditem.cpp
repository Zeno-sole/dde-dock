/*
 * Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *
 * Maintainer: sbw <sbw@sbw.so>
 *             listenerri <listenerri@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "constants.h"
#include "wireditem.h"
#include "util/horizontalseperator.h"
#include "../widgets/tipswidget.h"
#include "util/utils.h"
#include "util/statebutton.h"
#include "util/imageutil.h"

#include <DGuiApplicationHelper>

#include <QLabel>
#include <QPainter>
#include <QVBoxLayout>

#include <NetworkModel>

DGUI_USE_NAMESPACE

const int ItemHeight = 36;
extern const QString DarkType = "_dark.svg";
extern const QString LightType = ".svg";

WiredItem::WiredItem(WiredDevice *device, const QString &deviceName, QWidget *parent)
    : DeviceItem(device, parent)
    , m_deviceName(deviceName)
    , m_uuid(QString(device->info().value("UniqueUuid").toString()))
    , m_oldUUID(QString())
    , m_connectedName(new QLabel(this))
    , m_wiredIcon(new QLabel(this))
    , m_stateButton(new StateButton(this))
    , m_loadingStat(new DSpinner(this))
    , m_freshWiredIcon(new QTimer(this))
{
    setFixedHeight(ItemHeight);

    m_stateButton->setFixedSize(16, 16);
    m_stateButton->setType(StateButton::Check);
    m_stateButton->setVisible(false);
    m_loadingStat->setFixedSize(PLUGIN_ICON_MAX_SIZE, PLUGIN_ICON_MAX_SIZE);
    m_loadingStat->setVisible(false);

    m_connectedName->setText(m_deviceName);
    m_connectedName->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_connectedName->setForegroundRole(QPalette::BrightText);

    auto connectionLayout = new QVBoxLayout(this);
    connectionLayout->setMargin(0);
    connectionLayout->setSpacing(0);

    auto itemLayout = new QHBoxLayout;
    itemLayout->setMargin(0);
    itemLayout->setSpacing(0);
    itemLayout->addSpacing(28);
    itemLayout->addWidget(m_wiredIcon);
    itemLayout->addSpacing(8);
    itemLayout->addWidget(m_connectedName);
    itemLayout->addWidget(m_stateButton);
    itemLayout->addWidget(m_loadingStat);
    itemLayout->addSpacing(11);
    connectionLayout->addLayout(itemLayout);
    setLayout(connectionLayout);

    connect(m_freshWiredIcon, &QTimer::timeout, this, &WiredItem::setWiredStateIcon);
    connect(m_device, static_cast<void (NetworkDevice::*)(const bool) const>(&NetworkDevice::enableChanged),
            this, &WiredItem::enableChanged);
    connect(m_device, static_cast<void (NetworkDevice::*)(NetworkDevice::DeviceStatus) const>(&NetworkDevice::statusChanged),
            this, &WiredItem::deviceStateChanged);
    connect(m_device, static_cast<void (NetworkDevice::*)(NetworkDevice::DeviceStatus) const>(&NetworkDevice::statusChanged),
            this, &WiredItem::setWiredStateIcon);
    connect(m_device, static_cast<void (NetworkDevice::*)(bool) const>(&NetworkDevice::enableChanged),
            this, &WiredItem::setWiredStateIcon);

    connect(static_cast<WiredDevice *>(m_device.data()), &WiredDevice::activeWiredConnectionInfoChanged,
            this, &WiredItem::changedActiveWiredConnectionInfo);
    connect(static_cast<WiredDevice *>(m_device.data()), &WiredDevice::connectionsChanged,
            this, &WiredItem::changedConnections);

    connect(m_stateButton, &StateButton::click, this, [&] {
        auto enableState = m_device->enabled();
        emit requestSetDeviceEnable(path(), !enableState);
    });

    deviceStateChanged(m_device->status());
    setWiredStateIcon();
}

void WiredItem::setTitle(const QString &name)
{
    if (m_device->status() != NetworkDevice::Activated)
        m_connectedName->setText(name);
    m_deviceName = name;
}

bool WiredItem::deviceEabled()
{
    return m_device->enabled();
}

void WiredItem::setDeviceEnabled(bool enabled)
{
    emit requestSetDeviceEnable(path(), enabled);
}

WiredItem::WiredStatus WiredItem::getDeviceState()
{
    if (!m_device->enabled()) {
        return Disabled;
    }
    if (m_device->status() == NetworkDevice::Activated
            && NetworkModel::connectivity() != Connectivity::Full) {
        return ConnectNoInternet;
    }
    if (m_device->obtainIpFailed()) {
        return ObtainIpFailed;
    }

    switch (m_device->status()) {
        case NetworkDevice::Unknown:       return Unknown;
        case NetworkDevice::Unmanaged:
        case NetworkDevice::Unavailable:   return Nocable;
        case NetworkDevice::Disconnected:  return Disconnected;
        case NetworkDevice::Prepare:
        case NetworkDevice::Config:        return Connecting;
        case NetworkDevice::NeedAuth:      return Authenticating;
        case NetworkDevice::IpConfig:
        case NetworkDevice::IpCheck:
        case NetworkDevice::Secondaries:   return ObtainingIP;
        case NetworkDevice::Activated:     return Connected;
        case NetworkDevice::Deactivation:
        case NetworkDevice::Failed:        return Failed;
    }
    Q_UNREACHABLE();
}

QJsonObject WiredItem::getActiveWiredConnectionInfo()
{
    return static_cast<WiredDevice *>(m_device.data())->activeWiredConnectionInfo();
}

void WiredItem::setThemeType(DGuiApplicationHelper::ColorType themeType)
{
    bool isLight = (themeType == DGuiApplicationHelper::LightType);

    auto pixpath = QString(":/wired/resources/wired/network-wired-symbolic");
    pixpath = isLight ? pixpath + "-dark.svg" : pixpath +  LightType;
    auto iconPix = Utils::renderSVG(pixpath, QSize(PLUGIN_ICON_MAX_SIZE, PLUGIN_ICON_MAX_SIZE), devicePixelRatioF());
    m_wiredIcon->setPixmap(iconPix);
}

void WiredItem::setWiredStateIcon()
{
    QPixmap pixmap;
    QString iconString;
    QString stateString;

    auto ratio =  devicePixelRatioF();

    switch (m_deviceState) {
        case NetworkDevice::Unknown:
        case NetworkDevice::Unmanaged:
        case NetworkDevice::Unavailable: {
            stateString = "error";
            iconString = QString("network-%1-symbolic").arg(stateString);
        }
            break;
        case NetworkDevice::Disconnected: {
            stateString = "none";
            iconString = QString("network-%1-symbolic").arg(stateString);
        }
            break;
        case NetworkDevice::Deactivation:
        case NetworkDevice::Failed: {
            stateString = "offline";
            iconString = QString("network-%1-symbolic").arg(stateString);
        }
            break;
        case NetworkDevice::Prepare:
        case NetworkDevice::Config:
        case NetworkDevice::NeedAuth:
        case NetworkDevice::IpConfig:
        case NetworkDevice::IpCheck:
        case NetworkDevice::Secondaries: {
            m_freshWiredIcon->start(200);
            const int index = QTime::currentTime().msec() / 200 % 10;
            const int num = index + 1;
            qDebug() << num;
            iconString = QString("network-wired-symbolic-connecting%1").arg(num);
            if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType)
                iconString.append(PLUGIN_MIN_ICON_NAME);
            pixmap = ImageUtil::loadSvg(iconString, ":/", PLUGIN_ICON_MAX_SIZE, ratio);
            m_wiredIcon->setPixmap(pixmap);
            update();
            return;
        }
        case NetworkDevice::Activated: {
            stateString = "online";
            iconString = QString("network-%1-symbolic").arg(stateString);
        }
            break;
    }

    m_freshWiredIcon->stop();

    if (m_deviceState == NetworkDevice::Activated && NetworkModel::connectivity() != Connectivity::Full) {
        stateString = "warning";
        iconString = QString("network-%1-symbolic").arg(stateString);
    }

    if (!m_device->enabled()) {
        stateString = "disabled";
        iconString = QString("network-%1-symbolic").arg(stateString);
    }

    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType)
        iconString.append(PLUGIN_MIN_ICON_NAME);

    pixmap = ImageUtil::loadSvg(iconString, ":/", PLUGIN_ICON_MAX_SIZE, ratio);
    m_wiredIcon->setPixmap(pixmap);
    update();
}

void WiredItem::refreshConnectivity()
{
    setWiredStateIcon();
}

QSize WiredItem::sizeHint() const
{
    return QSize(DeviceItem::sizeHint().width(), ItemHeight);
}

void WiredItem::enterEvent(QEvent *event)
{
    m_isEnter = true;
    update();
    DeviceItem::enterEvent(event);
}

void WiredItem::leaveEvent(QEvent *event)
{
    m_isEnter = false;
    update();
    DeviceItem::leaveEvent(event);
}

void WiredItem::paintEvent(QPaintEvent *event)
{
    DeviceItem::paintEvent(event);

    QPainter painter(this);
    painter.setPen(Qt::NoPen);
    if (DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::LightType) {
        if (m_isEnter) {
            painter.setBrush(QColor(0, 0, 0, 0.12*255));
        } else {
            painter.setBrush(Qt::transparent);
        }
    } else {
        if (m_isEnter) {
            painter.setBrush(QColor(255, 255, 255, 0.12*255));
        } else {
            painter.setBrush(Qt::transparent);
        }
    }

    painter.drawRect(rect());
}

void WiredItem::mouseReleaseEvent(QMouseEvent *event)
{
    // 如果网络是连接状态，则不需要重新连接
    if (m_deviceState == NetworkDevice::Activated)
        return;

    emit requestActiveConnection(path(), m_uuid);

    DeviceItem::mouseReleaseEvent(event);
}

void WiredItem::deviceStateChanged(NetworkDevice::DeviceStatus state)
{
    m_deviceState = state;
    switch (state) {
        case NetworkDevice::Unknown:
        case NetworkDevice::Unmanaged:
        case NetworkDevice::Unavailable:
        case NetworkDevice::Disconnected:
        case NetworkDevice::Deactivation:
        case NetworkDevice::Failed: {
            m_loadingStat->stop();
            m_loadingStat->hide();
            m_loadingStat->setVisible(false);
            if (!m_device->enabled())
                m_stateButton->setVisible(false);
        }
        break;
        case NetworkDevice::Prepare:
        case NetworkDevice::Config:
        case NetworkDevice::NeedAuth:
        case NetworkDevice::IpConfig:
        case NetworkDevice::IpCheck:
        case NetworkDevice::Secondaries: {
            m_stateButton->setVisible(false);
            m_loadingStat->setVisible(true);
            m_loadingStat->start();
        }
        break;
        case NetworkDevice::Activated: {
            m_loadingStat->stop();
            m_loadingStat->setVisible(false);
            m_stateButton->setVisible(true);
        }
        break;
    }

    emit wiredStateChanged();
}

void WiredItem::changedActiveWiredConnectionInfo(const QJsonObject &connInfo)
{
    if (connInfo.isEmpty()) {
        m_stateButton->setVisible(false);
    } else {
        m_stateButton->setVisible(true);
        m_loadingStat->stop();
        m_loadingStat->setVisible(false);
    }

    emit activeConnectionChanged();
}

void WiredItem::changedConnections(const QList<QJsonObject> &connInfos)
{
    for (auto connInfo : connInfos) {
        QString uuid = connInfo.value("ConnectionUuid").toString();
        if (uuid == m_uuid ) {
            auto strTitle = connInfo.value("Id").toString();
            m_connectedName->setText(strTitle);
            QFontMetrics fontMetrics(m_connectedName->font());
            if (fontMetrics.width(strTitle) > m_connectedName->width()) {
                strTitle = QFontMetrics(m_connectedName->font()).elidedText(strTitle, Qt::ElideRight, m_connectedName->width());
            }
            if (strTitle.isEmpty())
                m_connectedName->setText(m_deviceName);
            else
                m_connectedName->setText(strTitle);
        }
    }

    emit activeConnectionChanged();
}
