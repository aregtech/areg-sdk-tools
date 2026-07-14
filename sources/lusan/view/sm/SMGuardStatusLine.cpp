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
 *  \file        lusan/view/sm/SMGuardStatusLine.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard status line + provenance chips (v7 B1 5-6).
 *
 ************************************************************************/

#include "lusan/view/sm/SMGuardStatusLine.hpp"

#include <QFontMetrics>
#include <QLabel>
#include <QResizeEvent>
#include <QVBoxLayout>

namespace
{
    QString severityWord(NEGuardStyle::eSeverity severity)
    {
        switch (severity)
        {
        case NEGuardStyle::eSeverity::Ok:   return QStringLiteral("ok");
        case NEGuardStyle::eSeverity::Warn: return QStringLiteral("warn");
        case NEGuardStyle::eSeverity::Err:
        default:                            return QStringLiteral("err");
        }
    }
}

SMGuardStatusLine::SMGuardStatusLine(QWidget* parent /*= nullptr*/)
    : QWidget   (parent)
    , mStatus   (nullptr)
    , mChips    (nullptr)
    , mSeverity (NEGuardStyle::eSeverity::Ok)
{
    QVBoxLayout* box = new QVBoxLayout(this);
    box->setContentsMargins(0, 0, 0, 0);
    box->setSpacing(2);

    mStatus = new QLabel(this);
    mStatus->setObjectName(QStringLiteral("smGuardStatus"));
    mStatus->setTextFormat(Qt::RichText);
    mStatus->setTextInteractionFlags(Qt::TextSelectableByMouse);
    box->addWidget(mStatus);

    mChips = new QLabel(this);
    mChips->setObjectName(QStringLiteral("smGuardChips"));
    box->addWidget(mChips);
}

void SMGuardStatusLine::setStatus(  NEGuardStyle::eSeverity severity
                                  , const QString& verdict
                                  , const QString& generatedPreview
                                  , const QStringList& handlerChips)
{
    if (verdict.isEmpty())
    {
        clearStatus();
        return;
    }

    mSeverity = severity;
    mVerdict  = verdict;
    mPreview  = generatedPreview;

    if (handlerChips.isEmpty())
    {
        mChips->hide();
    }
    else
    {
        const QColor color = NEGuardStyle::ownerColor(NEGuardStyle::eOwner::Handler);
        mChips->setText(QStringLiteral("<span style='color:%1;'>uses handler:</span> %2")
                            .arg(color.name(), handlerChips.join(QStringLiteral(", ")).toHtmlEscaped()));
        mChips->setToolTip(handlerChips.join(QStringLiteral("\n")));
        mChips->show();
    }

    updateLabel();
    show();
}

void SMGuardStatusLine::clearStatus()
{
    mVerdict.clear();
    mPreview.clear();
    mStatus->clear();
    mChips->hide();
    hide();
}

void SMGuardStatusLine::updateLabel()
{
    const QColor sevColor = NEGuardStyle::severityColor(mSeverity);
    QString html = QStringLiteral("<span style='color:%1; font-weight:bold;'>%2</span>&nbsp;&nbsp;%3")
                       .arg(sevColor.name(), severityWord(mSeverity), mVerdict.toHtmlEscaped());

    if (mPreview.isEmpty() == false)
    {
        const QString prefix = QStringLiteral(" -- generated: ");
        const int reserved = mStatus->fontMetrics().horizontalAdvance(severityWord(mSeverity) + QStringLiteral("  ") + mVerdict + prefix);
        const int avail = qMax(60, mStatus->width() - reserved - 8);
        const QString elided = mStatus->fontMetrics().elidedText(mPreview, Qt::ElideMiddle, avail);
        html += (prefix + QStringLiteral("<span style='font-family:monospace;'>%1</span>").arg(elided.toHtmlEscaped()));
        mStatus->setToolTip(mPreview);
    }
    else
    {
        mStatus->setToolTip(QString());
    }

    mStatus->setText(html);
}

void SMGuardStatusLine::resizeEvent(QResizeEvent* event)
{
    QWidget::resizeEvent(event);
    if (mVerdict.isEmpty() == false)
    {
        updateLabel();
    }
}
