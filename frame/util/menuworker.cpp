// Copyright (C) 2018 ~ 2020 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2018 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "menuworker.h"
#include "dockitemmanager.h"
#include "utils.h"
#include "displaymanager.h"

#include <QAction>
#include <QMenu>
#include <QGSettings>

#include <DApplication>
#include <DDBusSender>

#define DIS_INS DisplayManager::instance()

MenuWorker::MenuWorker(DockInter *dockInter, QObject *parent)
    : QObject(parent)
    , m_dockInter(dockInter)
{
}

void MenuWorker::createMenu(QMenu *settingsMenu)
{
    settingsMenu->setAccessibleName("settingsmenu");
    settingsMenu->setTitle("Settings Menu");

    // 模式
    const QGSettings *menuSettings = Utils::ModuleSettingsPtr("menu");
    if (!menuSettings || !menuSettings->keys().contains("modeVisible") || menuSettings->get("modeVisible").toBool()) {
        const DisplayMode displayMode = static_cast<DisplayMode>(m_dockInter->displayMode());

        QMenu *modeSubMenu = new QMenu(settingsMenu);
        modeSubMenu->setAccessibleName("modesubmenu");

        QAction *fashionModeAct = new QAction(tr("Fashion Mode"), this);
        QAction *efficientModeAct = new QAction(tr("Efficient Mode"), this);

        fashionModeAct->setCheckable(true);
        efficientModeAct->setCheckable(true);

        fashionModeAct->setChecked(displayMode == Fashion);
        efficientModeAct->setChecked(displayMode == Efficient);

        connect(fashionModeAct, &QAction::triggered, this, [ = ]{ m_dockInter->setDisplayMode(DisplayMode::Fashion); });
        connect(efficientModeAct, &QAction::triggered, this, [ = ]{ m_dockInter->setDisplayMode(DisplayMode::Efficient); });

        modeSubMenu->addAction(fashionModeAct);
        modeSubMenu->addAction(efficientModeAct);

        QAction *act = new QAction(tr("Mode"), this);
        act->setMenu(modeSubMenu);

        settingsMenu->addAction(act);
    }

    // 位置
    if (!menuSettings || !menuSettings->keys().contains("locationVisible") || menuSettings->get("locationVisible").toBool()) {
        const Position position = static_cast<Position>(m_dockInter->position());

        QMenu *locationSubMenu = new QMenu(settingsMenu);
        locationSubMenu->setAccessibleName("locationsubmenu");

        QAction *topPosAct = new QAction(tr("Top"), this);
        QAction *bottomPosAct = new QAction(tr("Bottom"), this);
        QAction *leftPosAct = new QAction(tr("Left"), this);
        QAction *rightPosAct = new QAction(tr("Right"), this);

        topPosAct->setCheckable(true);
        bottomPosAct->setCheckable(true);
        leftPosAct->setCheckable(true);
        rightPosAct->setCheckable(true);

        topPosAct->setChecked(position == Top);
        bottomPosAct->setChecked(position == Bottom);
        leftPosAct->setChecked(position == Left);
        rightPosAct->setChecked(position == Right);

        connect(topPosAct, &QAction::triggered, this, [ = ]{ m_dockInter->setPosition(Top); });
        connect(bottomPosAct, &QAction::triggered, this, [ = ]{ m_dockInter->setPosition(Bottom); });
        connect(leftPosAct, &QAction::triggered, this, [ = ]{ m_dockInter->setPosition(Left); });
        connect(rightPosAct, &QAction::triggered, this, [ = ]{ m_dockInter->setPosition(Right); });

        locationSubMenu->addAction(topPosAct);
        locationSubMenu->addAction(bottomPosAct);
        locationSubMenu->addAction(leftPosAct);
        locationSubMenu->addAction(rightPosAct);

        QAction *act = new QAction(tr("Location"), this);
        act->setMenu(locationSubMenu);

        settingsMenu->addAction(act);
    }

    // 状态
    if (!menuSettings || !menuSettings->keys().contains("statusVisible") || menuSettings->get("statusVisible").toBool()) {
        const HideMode hideMode = static_cast<HideMode>(m_dockInter->hideMode());

        QMenu *statusSubMenu = new QMenu(settingsMenu);
        statusSubMenu->setAccessibleName("statussubmenu");

        QAction *keepShownAct = new QAction(tr("Keep Shown"), this);
        QAction *keepHiddenAct = new QAction(tr("Keep Hidden"), this);
        QAction *smartHideAct = new QAction(tr("Smart Hide"), this);

        keepShownAct->setCheckable(true);
        keepHiddenAct->setCheckable(true);
        smartHideAct->setCheckable(true);

        keepShownAct->setChecked(hideMode == KeepShowing);
        keepHiddenAct->setChecked(hideMode == KeepHidden);
        smartHideAct->setChecked(hideMode == SmartHide);

        connect(keepShownAct, &QAction::triggered, this, [ = ]{ m_dockInter->setHideMode(KeepShowing); });
        connect(keepHiddenAct, &QAction::triggered, this, [ = ]{ m_dockInter->setHideMode(KeepHidden); });
        connect(smartHideAct, &QAction::triggered, this, [ = ]{ m_dockInter->setHideMode(SmartHide); });

        statusSubMenu->addAction(keepShownAct);
        statusSubMenu->addAction(keepHiddenAct);
        statusSubMenu->addAction(smartHideAct);

        QAction *act = new QAction(tr("Status"), this);
        act->setMenu(statusSubMenu);

        settingsMenu->addAction(act);
    }

    // 任务栏配置
    if (!menuSettings || !menuSettings->keys().contains("settingVisible") || menuSettings->get("settingVisible").toBool()) {
        QAction *act = new QAction(tr("Dock settings"), this);
        connect(act, &QAction::triggered, this, &MenuWorker::onDockSettingsTriggered);
        settingsMenu->addAction(act);
    }

    delete menuSettings;
}

void MenuWorker::onDockSettingsTriggered()
{
    DDBusSender().service(controllCenterService)
            .path(controllCenterPath)
            .interface(controllCenterInterface)
            .method("ShowPage")
            .arg(QString("personalization/desktop/dock"))
            .call();
}

void MenuWorker::exec()
{
    // 菜单功能被禁用
    static const QGSettings *setting = Utils::ModuleSettingsPtr("menu", QByteArray());
    if (setting && setting->keys().contains("enable") && !setting->get("enable").toBool()) {
        return;
    }

    QMenu menu;
    if (Utils::IS_WAYLAND_DISPLAY)
        menu.setWindowFlag(Qt::FramelessWindowHint);
    createMenu(&menu);
    menu.exec(QCursor::pos());
}
