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
 *  \file        lusan/view/log/LogEmptyState.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, what an empty log table says.
 *
 ************************************************************************/

#include "lusan/view/log/LogEmptyState.hpp"

#include "lusan/common/NELogPalette.hpp"
#include "lusan/common/NELusanCommon.hpp"

#include <QFont>
#include <QLabel>
#include <QLocale>
#include <QPushButton>
#include <QVBoxLayout>

namespace
{
    //! The glyph is drawn large, because it carries the reason before the words are read.
    constexpr int   _markExtent { 40 };
}

LogEmptyState::LogEmptyState(QWidget* parent /*= nullptr*/)
    : QWidget   (parent)

    , mMark     (nullptr)
    , mHeadline (nullptr)
    , mDetails  (nullptr)
    , mAction   (nullptr)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(24, 24, 24, 24);
    layout->setSpacing(8);
    layout->addStretch(1);

    mMark = new QLabel(this);
    mMark->setAlignment(Qt::AlignmentFlag::AlignCenter);
    layout->addWidget(mMark);

    mHeadline = new QLabel(this);
    mHeadline->setAlignment(Qt::AlignmentFlag::AlignCenter);
    QFont headline{ mHeadline->font() };
    headline.setBold(true);
    mHeadline->setFont(headline);
    layout->addWidget(mHeadline);

    mDetails = new QLabel(this);
    mDetails->setAlignment(Qt::AlignmentFlag::AlignCenter);
    mDetails->setWordWrap(true);
    QPalette dimmed{ mDetails->palette() };
    dimmed.setColor(QPalette::ColorRole::WindowText, NELogPalette::textColor(NELogPalette::eLogColorRole::RoleNotset));
    mDetails->setPalette(dimmed);
    layout->addWidget(mDetails);

    mAction = new QPushButton(tr("Clear filters"), this);
    mAction->setVisible(false);
    connect(mAction, &QPushButton::clicked, this, [this]() { emit signalClearFilters(); });

    QHBoxLayout* actionRow = new QHBoxLayout();
    actionRow->addStretch(1);
    actionRow->addWidget(mAction);
    actionRow->addStretch(1);
    layout->addLayout(actionRow);

    layout->addStretch(1);
    setVisible(false);
}

void LogEmptyState::setReason(LogEmptyState::eEmptyReason reason, int held, bool scopes, bool columns)
{
    if (reason == eEmptyReason::ReasonNone)
    {
        setVisible(false);
        return;
    }

    QIcon   mark;
    QString headline;
    QString details;

    switch (reason)
    {
    case eEmptyReason::ReasonNotConnected:
        mark     = NELusanCommon::iconLiveLogDisconnected(NELusanCommon::SizeBig);
        headline = tr("No log collector connected");
        details  = tr("Connect to a collector in the Live Logs navigation panel, and the logs of every\n"
                      "target it serves arrive here.");
        break;

    case eEmptyReason::ReasonNoArchive:
        mark     = NELusanCommon::iconOfflineLogWindow(NELusanCommon::SizeBig);
        headline = tr("No archive open");
        details  = tr("Open a log file in the Offline Logs navigation panel to read logs that were\n"
                      "collected earlier.");
        break;

    case eEmptyReason::ReasonNoLiveLogs:
        mark     = NELusanCommon::iconLiveLogConnected(NELusanCommon::SizeBig);
        headline = tr("Connected, and no log has arrived yet");
        details  = tr("The first log appears here as soon as a target produces one. A target that stays\n"
                      "silent may have every scope switched off; the Live Logs panel says which.");
        break;

    case eEmptyReason::ReasonEmptyArchive:
        mark     = NELusanCommon::iconOfflineLogWindow(NELusanCommon::SizeBig);
        headline = tr("This archive holds no log");
        details  = tr("The file was opened and read, and there was nothing in it.");
        break;

    case eEmptyReason::ReasonFiltered:
    default:
        mark     = NELusanCommon::iconFilter(NELusanCommon::SizeBig);
        headline = tr("%1 rows, and the filters keep out every one").arg(QLocale::system().toString(held));
        if (columns && scopes)
        {
            details = tr("Column filters are on and the navigation tree is hiding scopes.\n"
                         "Dropping them all brings every row back.");
        }
        else if (columns)
        {
            details = tr("Reopen a column filter from its header, or drop them all at once.");
        }
        else
        {
            details = tr("Every row comes from a scope the navigation tree is hiding.\n"
                         "Dropping the filters shows those scopes again.");
        }

        break;
    }

    mMark->setPixmap(mark.pixmap(_markExtent, _markExtent));
    mHeadline->setText(headline);
    mDetails->setText(details);
    mAction->setVisible((reason == eEmptyReason::ReasonFiltered) && (columns || scopes));
    setVisible(true);
    raise();
}
