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
 *  \file        lusan/app/LusanApplication.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application object for managing GUI-related functionality.
 *
 ************************************************************************/

#include "lusan/app/LusanApplication.hpp"
#include "lusan/data/common/WorkspaceEntry.hpp"

LusanApplication * LusanApplication::theApp{nullptr};

LusanApplication::LusanApplication(int argc, char* argv[])
    : QApplication  (argc, argv)
    , mOptions      ( )
{
    LusanApplication::theApp = this;
}

LusanApplication::~LusanApplication(void)
{
    LusanApplication::theApp = nullptr;
}

OptionsManager& LusanApplication::getOptions(void)
{
    Q_ASSERT(LusanApplication::theApp != nullptr);
    return LusanApplication::theApp->mOptions;
}

LusanApplication& LusanApplication::getApplication(void)
{
    Q_ASSERT(LusanApplication::theApp != nullptr);
    return (*LusanApplication::theApp);
}

WorkspaceEntry LusanApplication::getActiveWorkspace(void)
{
    Q_ASSERT(LusanApplication::theApp != nullptr);
    return LusanApplication::theApp->mOptions.getActiveWorkspace();
}

bool LusanApplication::isInitialized(void)
{
    return (LusanApplication::theApp != nullptr);
}
