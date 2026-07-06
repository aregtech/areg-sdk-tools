/************************************************************************
 *  This file is part of the Lusan project, an official component of the Areg SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the Areg Framework.
 *
 *  Lusan is available as free and open-source software under the Apache version 2.0 License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/view/si/SIMethodList.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
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
    
    QHeaderView* header = table->header();
    Q_ASSERT(header != nullptr);
    header->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    header->setSectionResizeMode(3, QHeaderView::Stretch);
}

QToolButton * SIMethodList::ctrlButtonAdd() const
{
    return ui->toolAddMethod;
}

QToolButton * SIMethodList::ctrlButtonInsert() const
{
    return ui->toolInsertMethod;
}

QToolButton * SIMethodList::ctrlButtonRemove() const
{
    return ui->toolDeleteMethod;
}

QToolButton * SIMethodList::ctrlButtonParamAdd() const
{
    return ui->toolParamAdd;
}

QToolButton * SIMethodList::ctrlButtonParamRemove() const
{
    return ui->toolParamDelete;
}

QToolButton * SIMethodList::ctrlButtonParamInsert() const
{
    return ui->toolParamInsert;
}

QToolButton * SIMethodList::ctrlButtonMoveUp() const
{
    return ui->toolMoveUp;
}

QToolButton * SIMethodList::ctrlButtonMoveDown() const
{
    return ui->toolMoveDown;
}

QTreeWidget * SIMethodList::ctrlTableList() const
{
    return ui->treeMethods;
}
