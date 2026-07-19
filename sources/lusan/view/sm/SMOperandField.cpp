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
 *  \file        lusan/view/sm/SMOperandField.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard smart operand field (v7 B6).
 *
 ************************************************************************/

#include "lusan/view/sm/SMOperandField.hpp"

#include "lusan/model/sm/SMTypeCompat.hpp"

#include <QCompleter>
#include <QLineEdit>

SMOperandField::SMOperandField(QWidget* parent /*= nullptr*/)
    : QComboBox         (parent)
    , mLastCommitted    ( )
    , mSilent           (false)
{
    setEditable(true);
    setInsertPolicy(QComboBox::NoInsert);
    setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
    setMinimumContentsLength(14);

    connect(lineEdit(), &QLineEdit::returnPressed, this, &SMOperandField::emitCommitted);
    connect(this, QOverload<int>::of(&QComboBox::activated), this, [this](int index)
    {
        // A picked row inserts the bare name, not the decorated label.
        const QString name = itemData(index).toString();
        if (name.isEmpty() == false)
        {
            setEditText(name);
        }

        emitCommitted();
    });
}

void SMOperandField::setSymbols(const QList<SMGuardSymbol>& symbols, const QString& targetType)
{
    const QString keep = currentText();

    mSilent = true;
    clear();

    // Exact-type candidates first, convertible with a `(!)` cue second, the rest after a
    // literal hint -- the same ranking the B5 slot popup uses.
    QList<const SMGuardSymbol*> exact;
    QList<const SMGuardSymbol*> convertible;
    QList<const SMGuardSymbol*> others;
    for (const SMGuardSymbol& sym : symbols)
    {
        if (targetType.isEmpty())
        {
            others.append(&sym);
            continue;
        }

        switch (SMTypeCompat::rank(sym.typeText, targetType))
        {
        case SMTypeCompat::eRank::Exact:    exact.append(&sym);         break;
        case SMTypeCompat::eRank::Converts: convertible.append(&sym);   break;
        default:                            others.append(&sym);        break;
        }
    }

    for (const SMGuardSymbol* sym : exact)
    {
        addItem(sym->glyph + QLatin1Char(' ') + sym->name + QStringLiteral("    ") + sym->typeText, sym->name);
    }
    for (const SMGuardSymbol* sym : convertible)
    {
        addItem(sym->glyph + QLatin1Char(' ') + sym->name + QStringLiteral("    (!) ") + sym->typeText, sym->name);
    }
    for (const SMGuardSymbol* sym : others)
    {
        addItem(sym->glyph + QLatin1Char(' ') + sym->name + QStringLiteral("    ") + sym->typeText, sym->name);
    }

    QStringList words;
    for (const SMGuardSymbol& sym : symbols)
    {
        words.append(sym.name);
    }

    QCompleter* completer = new QCompleter(words, this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);
    setCompleter(completer);

    setEditText(keep);
    mSilent = false;
}

void SMOperandField::setTextSilent(const QString& text)
{
    mSilent = true;
    setEditText(text);
    mLastCommitted = text;
    mSilent = false;
}

void SMOperandField::focusOutEvent(QFocusEvent* event)
{
    QComboBox::focusOutEvent(event);
    emitCommitted();
}

void SMOperandField::emitCommitted()
{
    if (mSilent)
    {
        return;
    }

    const QString text = currentText().trimmed();
    if ((text.isEmpty() == false) && (text != mLastCommitted))
    {
        mLastCommitted = text;
        emit committed(text);
    }
}
