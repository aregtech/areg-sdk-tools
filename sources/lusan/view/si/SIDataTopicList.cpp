﻿/************************************************************************
 *  This file is part of the Lusan project, an official component of the AREG SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the AREG Framework.
 *
 *  Lusan is available as free and open-source software under the MIT License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]aregtech.com.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/view/si/SIDataTopicList.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/
#include "lusan/view/si/SIDataTopicList.hpp"
#include "lusan/view/si/SICommon.hpp"
#include "ui/ui_SIDataTopicList.h"

SIDataTopicList::SIDataTopicList(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::SIDataTopicList)
{
    QFont font{ this->font() };
    font.setBold(false);
    font.setItalic(false);
    font.setPointSize(10);
    this->setFont(font);
    ui->setupUi(this);
    QTableWidget* table = ui->tableTopics;
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    setBaseSize(SICommon::WIDGET_WIDTH, SICommon::WIDGET_HEIGHT);
    setMinimumSize(SICommon::WIDGET_WIDTH, SICommon::WIDGET_HEIGHT);
}

QToolButton* SIDataTopicList::ctrlButtonAdd(void)
{
    return ui->toolAddTopic;
}

QToolButton* SIDataTopicList::ctrlButtonRemove(void)
{
    return ui->toolDeleteTopic;
}

QToolButton* SIDataTopicList::ctrlButtonInsert(void)
{
    return ui->toolInsertTopic;
}

QToolButton* SIDataTopicList::ctrlButtonMoveUp(void)
{
    return ui->toolMoveUp;
}

QToolButton* SIDataTopicList::ctrlButtonMoveDown(void)
{
    return ui->toolMoveDown;
}

QTableWidget* SIDataTopicList::ctrlTableList(void)
{
    return ui->tableTopics;
}
