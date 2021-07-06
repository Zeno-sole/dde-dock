/*
 * Copyright (C) 2018 ~ 2020 Uniontech Technology Co., Ltd.
 *
 * Author:     chenjun <chenjun@uniontech.com>
 *
 * Maintainer: chenjun <chenjun@uniontech.com>
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

#include <QObject>

#include <gtest/gtest.h>

#include "appdrag.h"

class Test_AppDrag : public ::testing::Test
{
public:
    virtual void SetUp() override;
    virtual void TearDown() override;
};

void Test_AppDrag::SetUp()
{
}

void Test_AppDrag::TearDown()
{
}

TEST_F(Test_AppDrag, drag_test)
{
    QWidget *w = new QWidget;
    AppDrag *drag = new AppDrag(w);
    QPixmap pix(":/res/all_settings_on.png");
    drag->setPixmap(pix);

    ASSERT_TRUE(drag->appDragWidget());

    drag->exec();

    delete w;
    w = nullptr;
}
