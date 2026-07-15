#ifndef LUSAN_VIEW_SM_SMOPERANDFIELD_HPP
#define LUSAN_VIEW_SM_SMOPERANDFIELD_HPP
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
 *  \file        lusan/view/sm/SMOperandField.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard smart operand field (v7 B6): an editable
 *               combo over the symbol catalog, ranked by type fit against a target type.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QComboBox>

#include "lusan/view/sm/SMGuardCatalog.hpp"

#include <QList>
#include <QString>

/**
 * \class   SMOperandField
 * \brief   The smart operand field of the mapping grid and clause popover (B5/B6 ranking):
 *          an editable QComboBox listing the in-scope symbols -- exact-type candidates
 *          first, convertible second with a `(!)` cue, everything else after a literal
 *          hint -- while free text (a literal or any expression) stays allowed. Emits
 *          \ref committed on Enter / item pick / focus-out; the owner parses the text.
 **/
class SMOperandField : public QComboBox
{
    Q_OBJECT

public:
    explicit SMOperandField(QWidget* parent = nullptr);

    /**
     * \brief   Fills the drop-down from \p symbols ranked against \p targetType (empty
     *          type = plain catalog order, no ranking cues).
     **/
    void setSymbols(const QList<SMGuardSymbol>& symbols, const QString& targetType);

    //!< Sets the current text without emitting \ref committed.
    void setTextSilent(const QString& text);

signals:
    //!< The user committed the text (Enter, list pick, or focus-out with a change).
    void committed(const QString& text);

protected:
    void focusOutEvent(QFocusEvent* event) override;

private:
    void emitCommitted();

private:
    QString mLastCommitted;     //!< Suppresses no-change commits.
    bool    mSilent;            //!< True while programmatically setting the text.
};

#endif  // LUSAN_VIEW_SM_SMOPERANDFIELD_HPP
