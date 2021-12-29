/*
 * Copyright (C) 2011 ~ 2018 Deepin Technology Co., Ltd.
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

#include "mainwindow.h"
#include "accessible.h"
#include "dbusdockadaptors.h"
#include "utils.h"
#include "themeappicon.h"
#include "dockitemmanager.h"
#include "dockapplication.h"

#include <QAccessible>
#include <QDir>
#include <QStandardPaths>
#include <QDateTime>
#include <QDir>

#include <DApplication>
#include <DLog>
#include <DGuiApplicationHelper>

#include <unistd.h>
#include <string>
#include <sys/mman.h>
#include <stdio.h>
#include <time.h>
#include <execinfo.h>
#include <sys/stat.h>
#include <signal.h>

DWIDGET_USE_NAMESPACE
#ifdef DCORE_NAMESPACE
DCORE_USE_NAMESPACE
#else
DUTIL_USE_NAMESPACE
#endif

using namespace std;

/**
 * @brief IsSaveMode
 * @return 判断当前是否应该进入安全模式（安全模式下不加载插件）
 */
bool IsSaveMode(const QString &configPath)
{
    QSettings settings(configPath, QSettings::IniFormat);
    settings.beginGroup(qApp->applicationName());
    int collapseNum = settings.value("collapse").toInt();
    // 启动时会记录一次，所以崩溃三次后此数应为4
    if (collapseNum - 1 >= 3) {
        settings.remove(""); // 删除记录的数据
        settings.setValue("collapse", 0);
        settings.endGroup();
        settings.sync();
        return true;
    }
    return false;
}

void record_start(const QString &configPath)
{
    QSettings settings(configPath, QSettings::IniFormat);
    settings.beginGroup(qApp->applicationName());

    const QString &timeFormat = "yyyy-MM-dd hh:mm:ss:zzz";
    const QString &currentTime = QDateTime::currentDateTime().toString(timeFormat);

    int collapseNum = settings.value("collapse").toInt();
    /* 第一次崩溃或进入安全模式后的第一次崩溃，将时间重置 */
    if (collapseNum == 0) {
        settings.setValue("first_time", currentTime);
    }
    QDateTime lastDate = QDateTime::fromString(settings.value("first_time").toString(), timeFormat);

    /* 将当前崩溃时间与第一次崩溃时间比较，小于9分钟，记录一次崩溃；大于9分钟，覆盖之前的崩溃时间 */
    if (qAbs(lastDate.secsTo(QDateTime::currentDateTime())) < 9 * 60) {
        settings.setValue("collapse", collapseNum + 1);
        switch (collapseNum) {
        case 0:
            settings.setValue("first_time", currentTime);
            break;
        case 1:
            settings.setValue("second_time", currentTime);
            break;
        case 2:
            settings.setValue("third_time", currentTime);
            break;
        default:
            qDebug() << "Error, the collapse is wrong!";
            break;
        }
    } else {
        if (collapseNum == 2){
            settings.setValue("first_time", settings.value("second_time").toString());
            settings.setValue("second_time", currentTime);
        } else {
            settings.setValue("first_time", currentTime);
        }
    }

    settings.endGroup();
    settings.sync();
}

int main(int argc, char *argv[])
{
    if (QString(getenv("XDG_CURRENT_DESKTOP")).compare("deepin", Qt::CaseInsensitive) == 0) {
        qDebug() << "Warning: force enable D_DXCB_FORCE_NO_TITLEBAR now!";
        setenv("D_DXCB_FORCE_NO_TITLEBAR", "1", 1);
    }

    DGuiApplicationHelper::setUseInactiveColorGroup(false);
    DockApplication app(argc, argv);

    auto configs = QStandardPaths::standardLocations(QStandardPaths::ConfigLocation);
    const QString &cfgPath = configs.size() <= 0 ? QString() :configs.first() + "/dde-cfg.ini";
    record_start(cfgPath);

    app.setOrganizationName("deepin");
    app.setApplicationName("dde-dock");
    app.setApplicationDisplayName("DDE Dock");
    app.setApplicationVersion("2.0");
    app.loadTranslator();
    app.setAttribute(Qt::AA_EnableHighDpiScaling, true);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps, false);

    // 自动化标记由此开始
    QAccessible::installFactory(accessibleFactory);

    // load dde-network-utils translator
    QTranslator translator;
    translator.load("/usr/share/dde-network-utils/translations/dde-network-utils_" + QLocale::system().name());
    app.installTranslator(&translator);

    // 设置日志输出到控制台以及文件
    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    // 启动入参 dde-dock --help可以看到一下内容， -x不加载插件 -r 一般用在startdde启动任务栏
    QCommandLineOption disablePlugOption(QStringList() << "x" << "disable-plugins", "do not load plugins.");
    QCommandLineOption runOption(QStringList() << "r" << "run-by-stardde", "run by startdde.");
    QCommandLineParser parser;
    parser.setApplicationDescription("DDE Dock");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addOption(disablePlugOption);
    parser.addOption(runOption);
    parser.process(app);

    // 任务栏单进程限制
    DGuiApplicationHelper::setSingleInstanceInterval(-1);
    if (!app.setSingleInstance(QString("dde-dock_%1").arg(getuid()))) {
        qDebug() << "set single instance failed!";
        return -1;
    }

#ifndef QT_DEBUG
    QDir::setCurrent(QApplication::applicationDirPath());
#endif

    // 注册任务栏的DBus服务
    MainWindow mw;
    DBusDockAdaptors adaptor(&mw);
    QDBusConnection::sessionBus().registerService("com.deepin.dde.Dock");
    QDBusConnection::sessionBus().registerObject("/com/deepin/dde/Dock", "com.deepin.dde.Dock", &mw);

    // 当任务栏以-r参数启动时，设置CANSHOW未false，之后调用launch不显示任务栏
    qApp->setProperty("CANSHOW", !parser.isSet(runOption));

    mw.launch();

    // 判断是否进入安全模式，是否带有入参 -x
    if (!IsSaveMode(cfgPath) && !parser.isSet(disablePlugOption)) {
        DockItemManager::instance()->startLoadPlugins();
        qApp->setProperty("PLUGINSLOADED", true);
    } else {
        mw.sendNotifications();
    }

    return app.exec();
}
