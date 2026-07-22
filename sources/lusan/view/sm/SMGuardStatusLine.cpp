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
 *  \brief       Lusan application, FSM guard status line + provenance chips.
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
    // LinksAccessibleByMouse is required in addition to the selectable flag: without it the
    // recovery hyperlink renders but never fires linkActivated (R20).
    mStatus->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::LinksAccessibleByMouse);
    connect(mStatus, &QLabel::linkActivated, this, [this](const QString& href)
    {
        // The href encodes the fix as `fix:<id>:<payload>` (payload may be empty); split on the
        // first ':' after the scheme so a payload that itself contains ':' stays intact.
        if (href.startsWith(QStringLiteral("fix:")))
        {
            const QString rest = href.mid(4);
            const int sep = rest.indexOf(QLatin1Char(':'));
            const QString fixId   = (sep >= 0) ? rest.left(sep) : rest;
            const QString payload = (sep >= 0) ? rest.mid(sep + 1) : QString();
            emit suggestionActivated(fixId, payload);
        }
    });
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

    // The generated preview and the `uses handler:` chips are NO LONGER shown here:
    // they live in the `Generated` accordion section, which the developer opens only
    // when interested. The status line keeps exactly one job -- the verdict -- while \ref previewText
    // still reports the byte-exact generator output for the 23c contract.
    Q_UNUSED(handlerChips);
    mChips->hide();

    updateLabel();
    show();
}

void SMGuardStatusLine::setSuggestion(const QString& fixId, const QString& payload, const QString& label)
{
    if ((fixId == mSugFixId) && (payload == mSugPayload) && (label == mSugLabel))
    {
        return;
    }

    mSugFixId   = fixId;
    mSugPayload = payload;
    mSugLabel   = label;

    // Only re-render while a verdict is shown; an empty guard keeps the line hidden.
    if (mVerdict.isEmpty() == false)
    {
        updateLabel();
    }
}

void SMGuardStatusLine::clearStatus()
{
    mVerdict.clear();
    mPreview.clear();
    mSugFixId.clear();
    mSugPayload.clear();
    mSugLabel.clear();
    mStatus->clear();
    mChips->hide();
    hide();
}

void SMGuardStatusLine::updateLabel()
{
    // `ok` alone when the guard is sound; `err: <what is wrong>` / `warn: <what is risky>` when it
    // is not -- the severity key carries the state and the message carries the detail, nothing else.
    const QColor sevColor = NEGuardStyle::severityColor(mSeverity);
    QString html = QStringLiteral("<span style='color:%1; font-weight:bold;'>%2</span>")
                       .arg(sevColor.name(), severityWord(mSeverity));
    if (mSeverity == NEGuardStyle::eSeverity::Ok)
    {
        mStatus->setToolTip(QString());
    }
    else
    {
        html += QStringLiteral(": %1").arg(mVerdict.toHtmlEscaped());
        mStatus->setToolTip(mVerdict);
    }

    // The R20 recovery affordance: a trailing `  ->  <label>` hyperlink when a suggestion exists.
    // No inline color -- the anchor uses the palette's Link role, so it stays theme-correct and
    // carries no literal color. The href encodes the fix id + payload for the linkActivated route.
    if (mSugFixId.isEmpty() == false)
    {
        html += QStringLiteral("  -&gt;  <a href=\"fix:%1:%2\">%3</a>")
                    .arg(mSugFixId, mSugPayload.toHtmlEscaped(), mSugLabel.toHtmlEscaped());
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
