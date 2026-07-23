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
 *  \file        lusan/view/sm/SMFixBar.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard quick-fix bar.
 *
 ************************************************************************/

#include "lusan/view/sm/SMFixBar.hpp"

#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>

SMFixBar::SMFixBar(QWidget* parent /*= nullptr*/)
    : QWidget   (parent)
    , mLayout   (nullptr)
    , mMessage  (nullptr)
{
    setObjectName(QStringLiteral("smGuardFixBar"));
    mLayout = new QHBoxLayout(this);
    mLayout->setContentsMargins(0, 0, 0, 0);
    mLayout->setSpacing(6);

    mMessage = new QLabel(this);
    mMessage->setObjectName(QStringLiteral("smGuardFixMessage"));
    // The message wraps and never dictates a width (the Properties dock must not grow to fit it).
    mMessage->setWordWrap(true);
    mMessage->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    mLayout->addWidget(mMessage);

    hide();
}

QSize SMFixBar::minimumSizeHint() const
{
    // Report no minimum width: the fix row must never force the hosting panel wider than the
    // width the user gave it via the splitter; excess content clips.
    return QSize(0, QWidget::minimumSizeHint().height());
}

void SMFixBar::clearButtons()
{
    // Remove every widget after the message label.
    while (mLayout->count() > 1)
    {
        QLayoutItem* item = mLayout->takeAt(1);
        if (item->widget() != nullptr)
        {
            item->widget()->deleteLater();
        }

        delete item;
    }
}

void SMFixBar::setFixes(const QString& message, const QList<Fix>& fixes)
{
    if (fixes.isEmpty())
    {
        dismiss();
        return;
    }

    clearButtons();
    mMessage->setText(message);

    for (const Fix& fix : fixes)
    {
        QToolButton* button = new QToolButton(this);
        button->setText(fix.label);
        button->setToolTip(fix.tooltip);
        button->setEnabled(fix.enabled);
        button->setAutoRaise(true);
        button->setCursor(Qt::PointingHandCursor);
        mLayout->addWidget(button);

        const QString id = fix.id;
        const QString payload = fix.payload;
        connect(button, &QToolButton::clicked, this, [this, id, payload]()
        {
            emit triggered(id, payload);
        });
    }

    QToolButton* close = new QToolButton(this);
    close->setText(QStringLiteral("x"));
    close->setAutoRaise(true);
    close->setToolTip(QStringLiteral("Dismiss"));
    mLayout->addWidget(close);
    connect(close, &QToolButton::clicked, this, [this]()
    {
        dismiss();
        emit dismissed();
    });

    mLayout->addStretch(1);
    show();
}

void SMFixBar::dismiss()
{
    hide();
}
