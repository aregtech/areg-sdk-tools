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
 *  \file        lusan/view/si/SIConstantList.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/
#include "lusan/view/si/SIConstantList.hpp"
#include "lusan/view/si/SICommon.hpp"
#include "ui/ui_SIConstantList.h"

SIConstantList::SIConstantList(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::SIConstantList)
{
    QFont font{ this->font() };
    font.setBold(false);
    font.setItalic(false);
    font.setPointSize(10);
    this->setFont(font);
    ui->setupUi(this);
    QTableWidget* table = ui->tableConstants;
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    setBaseSize(SICommon::WIDGET_WIDTH, SICommon::WIDGET_HEIGHT);
    setMinimumSize(SICommon::WIDGET_WIDTH, SICommon::WIDGET_HEIGHT);
    
    QHeaderView* header = table->horizontalHeader();
    Q_ASSERT(header != nullptr);
    header->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(2, QHeaderView::Stretch);
}

QToolButton* SIConstantList::ctrlButtonAdd(void)
{
    return ui->toolAddElem;
}

QToolButton* SIConstantList::ctrlButtonRemove(void)
{
    return ui->toolDeleteElem;
}

QToolButton* SIConstantList::ctrlButtonInsert(void)
{
    return ui->toolInsertElem;
}

QToolButton* SIConstantList::ctrlButtonMoveUp(void)
{
    return ui->toolMoveUp;
}

QToolButton* SIConstantList::ctrlButtonMoveDown(void)
{
    return ui->toolMoveDown;
}

QTableWidget* SIConstantList::ctrlTableList(void)
{
    return ui->tableConstants;
}
