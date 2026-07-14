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
 *  \file        lusan/view/sm/SMSignatureCard.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard call signature card (v7 B5).
 *
 ************************************************************************/

#include "lusan/view/sm/SMSignatureCard.hpp"

#include <QLabel>
#include <QVBoxLayout>

SMSignatureCard::SMSignatureCard(QWidget* parent /*= nullptr*/)
    : QFrame    (parent)
    , mSignature(nullptr)
    , mActive   (nullptr)
{
    setObjectName(QStringLiteral("smSignatureCard"));
    setWindowFlags(Qt::ToolTip | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_ShowWithoutActivating);
    setFrameShape(QFrame::StyledPanel);

    QVBoxLayout* box = new QVBoxLayout(this);
    box->setContentsMargins(8, 6, 8, 6);
    box->setSpacing(2);

    mSignature = new QLabel(this);
    mSignature->setTextFormat(Qt::RichText);
    mSignature->setStyleSheet(QStringLiteral("font-family: monospace;"));
    box->addWidget(mSignature);

    mActive = new QLabel(this);
    box->addWidget(mActive);
}

void SMSignatureCard::showFor(const SMGuardSymbol& call, int activeParam, const QPoint& globalPos)
{
    // Build `name(param0 : type0, param1 : type1) -> ret -- provenance` with the active
    // parameter underlined.
    QString sig = call.name.toHtmlEscaped() + QStringLiteral("(");
    for (int i = 0; i < call.paramNames.size(); ++i)
    {
        if (i > 0)
        {
            sig += QStringLiteral(", ");
        }

        const QString piece = (call.paramNames.at(i) + QStringLiteral(" : ")
                               + (i < call.paramTypes.size() ? call.paramTypes.at(i) : QString())).toHtmlEscaped();
        sig += (i == activeParam) ? QStringLiteral("<u>%1</u>").arg(piece) : piece;
    }

    sig += QStringLiteral(")");
    if (call.typeText.isEmpty() == false)
    {
        sig += QStringLiteral(" -> ") + call.typeText.toHtmlEscaped();
    }
    if (call.provenance.isEmpty() == false)
    {
        sig += QStringLiteral(" -- ") + call.provenance.toHtmlEscaped();
    }

    mSignature->setText(sig);

    if ((activeParam >= 0) && (activeParam < call.paramNames.size()))
    {
        const QString type = (activeParam < call.paramTypes.size()) ? call.paramTypes.at(activeParam) : QString();
        mActive->setText(tr("%1 : %2 -- pick or type the value to pass")
                             .arg(call.paramNames.at(activeParam), type));
        mActive->show();
    }
    else
    {
        mActive->hide();
    }

    adjustSize();
    move(globalPos);
    if (isVisible() == false)
    {
        show();
    }
}
