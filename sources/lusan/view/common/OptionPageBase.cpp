#include "OptionPageBase.hpp"
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
 *  \file        lusan/view/common/OptionPageBase.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, options page dialog base class.
 *
 ************************************************************************/

#include "lusan/view/common/OptionPageBase.hpp"

#include <QDialog>

OptionPageBase::OptionPageBase(QDialog* parent)
    : QWidget       (parent)
    , mDataModified (false)
    , mCanSave      (true)
{
}

void OptionPageBase::applyChanges(void)
{
    mDataModified   = false;
    mCanSave        = true;
}

void OptionPageBase::closingOptions(bool /*OKpressed*/)
{
    mDataModified = false;
}

void OptionPageBase::warnMessage(void)
{
}

void OptionPageBase::updateWorkspaceDirectories(const sWorkspaceDir & sources, const sWorkspaceDir& includes, const sWorkspaceDir& delivery, const sWorkspaceDir& logs)
{
}
