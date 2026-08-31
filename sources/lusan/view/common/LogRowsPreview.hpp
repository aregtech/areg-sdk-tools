#ifndef LUSAN_VIEW_COMMON_LOGROWSPREVIEW_HPP
#define LUSAN_VIEW_COMMON_LOGROWSPREVIEW_HPP
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
 *  \file        lusan/view/common/LogRowsPreview.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the sample log rows shown in the settings.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QWidget>

#include "lusan/common/NELogPalette.hpp"

/**
 * \brief   Draws one sample row per priority the way the log table would draw it, in a
 *          chosen colour set and row height. Shows the reader what a setting does before
 *          it is applied.
 **/
class LogRowsPreview : public QWidget
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit LogRowsPreview(QWidget* parent = nullptr);

    virtual ~LogRowsPreview(void) = default;

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Sets what the sample rows are drawn with and repaints them.
     * \param   palette The colour set to draw the sample rows in.
     * \param   height  The height of one sample row, in pixels.
     **/
    void setSample(NELogPalette::eLogPalette palette, int height);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:

    virtual QSize sizeHint(void) const override;

    virtual void paintEvent(QPaintEvent* event) override;

//////////////////////////////////////////////////////////////////////////
// Hidden member variables
//////////////////////////////////////////////////////////////////////////
private:
    NELogPalette::eLogPalette   mPalette;   //!< The colour set the sample rows are drawn in.
    int                         mRowHeight; //!< The height of one sample row.
};

#endif  // LUSAN_VIEW_COMMON_LOGROWSPREVIEW_HPP
