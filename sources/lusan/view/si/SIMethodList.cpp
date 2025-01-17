/************************************************************************
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
 *  \file        lusan/view/si/SIMethodList.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/
#include "lusan/view/si/SIMethodList.hpp"
#include "lusan/view/si/SICommon.hpp"
#include "ui/ui_SIMethodList.h"

SIMethodList::SIMethodList(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SIMethodList)
{
    QFont font{this->font()};
    font.setBold(false);
    font.setItalic(false);
    font.setPointSize(10);
    this->setFont(font);
    ui->setupUi(this);
    QTreeWidget* table = ui->treeMethods;
    table->header()->setSectionResizeMode(QHeaderView::Stretch);
    setBaseSize(SICommon::WIDGET_WIDTH, SICommon::WIDGET_HEIGHT);
    setMinimumSize(SICommon::WIDGET_WIDTH, SICommon::WIDGET_HEIGHT);
}

QToolButton * SIMethodList::ctrlButtonAdd(void) const
{
    return ui->toolAddMethod;
}

QToolButton * SIMethodList::ctrlButtonRemove(void) const
{
    return ui->toolDeleteMethod;
}

QToolButton * SIMethodList::ctrlButtonParamAdd(void) const
{
    return ui->toolParamAdd;
}

QToolButton * SIMethodList::ctrlButtonParamRemove(void) const
{
    return ui->toolParamDelete;
}

QToolButton * SIMethodList::ctrlButtonParamInsert(void) const
{
    return ui->toolParamInsert;
}

QToolButton * SIMethodList::ctrlButtonMoveUp(void) const
{
    return ui->toolMoveUp;
}

QToolButton * SIMethodList::ctrlButtonMoveDown(void) const
{
    return ui->toolMoveDown;
}

QTreeWidget * SIMethodList::ctrlTableList(void) const
{
    return ui->treeMethods;
}
