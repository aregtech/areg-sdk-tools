#ifndef LUSAN_VIEW_COMMON_OPTIONPAGEDISPLAY_HPP
#define LUSAN_VIEW_COMMON_OPTIONPAGEDISPLAY_HPP
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
 *  \copyright   (c) 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/view/common/OptionPageDisplay.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the display settings page.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/view/common/OptionPageBase.hpp"

#include "lusan/common/NETimeUnits.hpp"

class QComboBox;
class QDialog;

/**
 * \brief   The settings that change how the application writes what it shows, rather than
 *          what it shows. Currently the unit every measured time is written in.
 **/
class OptionPageDisplay : public OptionPageBase
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit OptionPageDisplay(QDialog* parent);

    virtual ~OptionPageDisplay(void) = default;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Writes the chosen settings and puts them in effect.
     **/
    void applyChanges(void) override;

//////////////////////////////////////////////////////////////////////////
// Hidden calls
//////////////////////////////////////////////////////////////////////////
private:

    //!< Builds the controls of the page.
    void setupWidgets(void);

    //!< Fills the controls from the stored options.
    void refreshFromOptions(void);

    //!< Returns the unit the selector currently holds.
    NETimeUnits::eTimeUnit selectedUnit(void) const;

//////////////////////////////////////////////////////////////////////////
// Hidden member variables
//////////////////////////////////////////////////////////////////////////
private:
    QComboBox*  mTimeUnit;  //!< Chooses the unit every measured time is written in.

//////////////////////////////////////////////////////////////////////////
// Forbidden calls
//////////////////////////////////////////////////////////////////////////
private:
    OptionPageDisplay(void) = delete;
    AREG_NOCOPY_NOMOVE(OptionPageDisplay);
};

#endif  // LUSAN_VIEW_COMMON_OPTIONPAGEDISPLAY_HPP
