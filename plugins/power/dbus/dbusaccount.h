/*
 * Copyright (C) 2015 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     sbw <sbw@sbw.so>
 *
 * Maintainer: sbw <sbw@sbw.so>
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

/*
 * This file was generated by qdbusxml2cpp version 0.8
 * Command line was: qdbusxml2cpp -c DBusAccount -p dbusaccount com.deepin.daemon.Accounts.xml
 *
 * qdbusxml2cpp is Copyright (C) 2015 Digia Plc and/or its subsidiary(-ies).
 *
 * This is an auto-generated file.
 * Do not edit! All changes made to it will be lost.
 */

#ifndef DBUSACCOUNT_H_1439464396
#define DBUSACCOUNT_H_1439464396

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QVariant>
#include <QtDBus/QtDBus>

/*
 * Proxy class for interface com.deepin.daemon.Accounts
 */
class DBusAccount: public QDBusAbstractInterface
{
    Q_OBJECT

    Q_SLOT void __propertyChanged__(const QDBusMessage &msg)
    {
        QList<QVariant> arguments = msg.arguments();
        if (3 != arguments.count()) {
            return;
        }
        QString interfaceName = msg.arguments().at(0).toString();
        if (interfaceName != "com.deepin.daemon.Accounts") {
            return;
        }
        QVariantMap changedProps = qdbus_cast<QVariantMap>(arguments.at(1).value<QDBusArgument>());
        QStringList keys = changedProps.keys();
        foreach(const QString & prop, keys) {
            const QMetaObject *self = metaObject();
            for (int i = self->propertyOffset(); i < self->propertyCount(); ++i) {
                QMetaProperty p = self->property(i);
                if (p.name() == prop) {
                    Q_EMIT p.notifySignal().invoke(this);
                }
            }
        }
    }
public:
    static inline const char *staticService()
    { return "com.deepin.daemon.Accounts"; }
    static inline const char *staticInterfacePath()
    { return "/com/deepin/daemon/Accounts"; }
    static inline const char *staticInterfaceName()
    { return "com.deepin.daemon.Accounts"; }

public:
    DBusAccount(QObject *parent = 0);

    ~DBusAccount();

    Q_PROPERTY(QStringList UserList READ userList)
    inline QStringList userList() const
    { return qvariant_cast< QStringList >(property("UserList")); }
};

//namespace com {
//  namespace deepin {
//    namespace daemon {
//      typedef ::DBusAccount Accounts;
//    }
//  }
//}
#endif