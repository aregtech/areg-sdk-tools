#ifndef LUSAN_VIEW_SM_SMGUARDSTATUSLINE_HPP
#define LUSAN_VIEW_SM_SMGUARDSTATUSLINE_HPP
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
 *  \file        lusan/view/sm/SMGuardStatusLine.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard status line + provenance chips (v7 B1 5-6).
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QWidget>

#include "lusan/view/sm/NEGuardStyle.hpp"

#include <QString>
#include <QStringList>

/************************************************************************
 * Dependencies
 ************************************************************************/
class QLabel;

/**
 * \class   SMGuardStatusLine
 * \brief   The status line under the field (B1 item 5) and the provenance chip row (item 6):
 *          a severity word + type verdict + the `generated:` C++ preview (middle-elided, full
 *          text on tooltip), and a `uses handler: X, Y` chip. The generated segment is the
 *          verbatim \ref SMGuardCodegenPreview output the caller passes -- the status line
 *          never re-derives it, so it is byte-exact with the generator contract (item 23c).
 *          \ref previewText exposes that exact string for tests.
 **/
class SMGuardStatusLine : public QWidget
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMGuardStatusLine(QWidget* parent = nullptr);

//////////////////////////////////////////////////////////////////////////
// Operations
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Shows the status: a severity word/icon, a verdict phrase, the (optional)
     *          generated preview, and the handler chips. An empty verdict hides the widget
     *          (E0 quiet).
     **/
    void setStatus(  NEGuardStyle::eSeverity severity
                   , const QString& verdict
                   , const QString& generatedPreview
                   , const QStringList& handlerChips);

    //!< Hides the status line (E0 -- empty guard).
    void clearStatus();

    //!< The exact generated-preview string currently shown (for tests / 23c).
    inline const QString& previewText() const;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
protected:
    void resizeEvent(QResizeEvent* event) override;

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    void updateLabel();

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    QLabel*                 mStatus;    //!< The severity + verdict + generated preview line.
    QLabel*                 mChips;     //!< The `uses handler:` chip row.
    NEGuardStyle::eSeverity mSeverity;  //!< The current severity.
    QString                 mVerdict;   //!< The current verdict phrase.
    QString                 mPreview;   //!< The exact generated preview string.
};

//////////////////////////////////////////////////////////////////////////
// Inline methods
//////////////////////////////////////////////////////////////////////////

inline const QString& SMGuardStatusLine::previewText() const
{
    return mPreview;
}

#endif  // LUSAN_VIEW_SM_SMGUARDSTATUSLINE_HPP
