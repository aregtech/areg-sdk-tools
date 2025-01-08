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
 *  \file        lusan/view/si/SIDataTypeList.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/
#include "lusan/view/si/SIDataTypeList.hpp"
#include "lusan/view/si/SICommon.hpp"
#include "ui/ui_SIDataTypeList.h"

SIDataTypeList::SIDataTypeList(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SIDataTypeList)
{
    QFont font{this->font()};
    font.setBold(false);
    font.setItalic(false);
    font.setPointSize(10);
    this->setFont(font);
    ui->setupUi(this);
    QTableView* table = ui->tableTypes;
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    setBaseSize(SICommon::WIDGET_WIDTH, SICommon::WIDGET_HEIGHT);
    setMinimumSize(SICommon::WIDGET_WIDTH, SICommon::WIDGET_HEIGHT);
}

QTableView* SIDataTypeList::ctrlTableList(void) const
{
    return ui->tableTypes;
}

QToolButton* SIDataTypeList::ctrlButtonAdd(void) const
{
    return ui->toolAddType;
}

QToolButton* SIDataTypeList::ctrlButtonRemove(void) const
{
    return ui->toolDeleteType;
}

QToolButton* SIDataTypeList::ctrlButtonMoveUp(void) const
{
    return ui->toolMoveUp;
}

QToolButton* SIDataTypeList::ctrlButtonMoveDown(void) const
{
    return ui->toolMoveDown;
}

QToolButton* SIDataTypeList::ctrlButtonAddField(void) const
{
    return ui->toolFieldAdd;
}

QToolButton* SIDataTypeList::ctrlButtonRemoveField(void) const
{
    return ui->toolFieldDelete;
}

QToolButton* SIDataTypeList::ctrlButtonInsertField(void) const
{
    return ui->toolFieldInsert;
}
