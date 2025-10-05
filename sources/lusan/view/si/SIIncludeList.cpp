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
 *  \file        lusan/view/si/SIIncludeList.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/
#include "lusan/view/si/SIIncludeList.hpp"
#include "lusan/view/si/SICommon.hpp"
#include "ui/ui_SIIncludeList.h"

#include "lusan/view/si/SIIncludeDetails.hpp"
#include "lusan/model/si/SIIncludeModel.hpp"

SIIncludeList::SIIncludeList(SIIncludeModel& model, QWidget* parent)
    : QWidget   (parent)
    , ui        (new Ui::SIIncludeList)
    , mModel    (model)
{
    QFont font{ this->font() };
    font.setBold(false);
    font.setItalic(false);
    font.setPointSize(10);
    this->setFont(font);
    ui->setupUi(this);
    QTableWidget* table = ui->tableIncludes;
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    setBaseSize(SICommon::WIDGET_WIDTH, SICommon::WIDGET_HEIGHT);
    setMinimumSize(SICommon::WIDGET_WIDTH, SICommon::WIDGET_HEIGHT);

    QHeaderView* header = table->horizontalHeader();
    Q_ASSERT(header != nullptr);
    header->setSectionResizeMode(0, QHeaderView::Stretch);
}

QToolButton* SIIncludeList::ctrlButtonAdd(void) const
{
    return ui->toolAddElem;
}

QToolButton* SIIncludeList::ctrlButtonRemove(void) const
{
    return ui->toolDeleteElem;
}

QToolButton* SIIncludeList::ctrlButtonMoveUp(void) const
{
    return ui->toolMoveUp;
}

QToolButton* SIIncludeList::ctrlButtonMoveDown(void) const
{
    return ui->toolMoveDown;
}

QToolButton* SIIncludeList::ctrlButtonInsert(void) const
{
    return ui->toolInsertElem;
}

QTableWidget* SIIncludeList::ctrlTableList(void) const
{
    return ui->tableIncludes;
}
