#ifndef LUSAN_VIEW_COMMON_OPTIONPAGEBASE_HPP
#define LUSAN_VIEW_COMMON_OPTIONPAGEBASE_HPP
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
 *  \file        lusan/view/common/OptionPageBase.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, options page dialog base class.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QWidget>
#include "areg/base/GEMacros.h"

class OptionPageBase : public QWidget
{
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
protected:
    OptionPageBase(ProjectSettings* parent);

    virtual ~OptionPageBase(void) = default;

public:

    virtual bool applyChanges(void);

    virtual void closingOptions(bool OKpressed);

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    OptionPageBase(void) = delete;
    DECLARE_NOCOPY_NOMOVE(OptionPageBase);
};

#endif  // LUSAN_VIEW_COMMON_OPTIONPAGEBASE_HPP
