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
 *  \file        lusan/view/common/OutputDock.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Output window.
 *
 ************************************************************************/

#include "lusan/view/common/OutputDock.hpp"

#include "lusan/common/NELusanCommon.hpp"
#include "lusan/view/common/MdiMainWindow.hpp"
#include "lusan/view/sm/SMValidationPanel.hpp"
#include "lusan/view/sm/StateMachine.hpp"

#include <QLabel>
#include <QVBoxLayout>

#include <algorithm>

const QString   OutputDock::TabNameLogging{ tr("Log analyzes") };
const QString   OutputDock::TabNameValidation{ tr("Validation") };

const QString& OutputDock::getTabName(OutputDock::eOutputDock wndOutput)
{
    static const QString   _empty;
    switch (wndOutput)
    {
    case OutputDock::eOutputDock::OutputLogging:
        return OutputDock::TabNameLogging;
    case OutputDock::eOutputDock::OutputValidation:
        return OutputDock::TabNameValidation;
    default:
        return _empty;
    }
}

OutputDock::eOutputDock OutputDock::getOutputDock(const QString& tabName)
{
    if (tabName == OutputDock::TabNameLogging)
        return OutputDock::eOutputDock::OutputLogging;
    else if (tabName == OutputDock::TabNameValidation)
        return OutputDock::eOutputDock::OutputValidation;
    else
        return OutputDock::eOutputDock::OutputUnknown;
}

OutputDock::OutputDock(MdiMainWindow* parent)
    : QWidget       (parent)
    , mMainWindow   (parent)
    , mTabs         (this)
    , mScopeOutput  (parent, this)
    , mValidationTab(nullptr)
    , mValidationBody(nullptr)
    , mValidationView(nullptr)
    , mValidation   (nullptr)
    , mBoundDocs    ( )
{
    mTabs.addTab(&mScopeOutput, QIcon(), tr("Scopes Analyzes"));

    // The findings of the active State Machine document: errors, warnings and advisory notes,
    // one per row, worst severity first. The output window is where a build-style diagnostic
    // list belongs, so the findings live here rather than in a per-document panel.
    mValidationTab = new QWidget(this);
    mValidationTab->setObjectName(QStringLiteral("outputValidationTab"));
    mValidationBody = new QVBoxLayout(mValidationTab);
    mValidationBody->setContentsMargins(0, 0, 0, 0);
    mValidationBody->setSpacing(0);
    showValidationPlaceholder();
    mTabs.addTab(mValidationTab, QIcon(), OutputDock::TabNameValidation);

    mTabs.setTabPosition(QTabWidget::South);

    // The tab widget is this content widget's whole body; the hosting ADS dock provides the
    // title bar and frame (issue #516).
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(&mTabs);

    initSize();
}

OutputDock::~OutputDock()
{
    mTabs.removeTab(0);
    mTabs.setParent(nullptr);
}

void OutputDock::initSize()
{
    // Expanding vertically on purpose. A Fixed policy here makes the content's maximum height
    // equal its hint, which propagates to the ADS dock area: the splitter between the editor
    // and this dock then has min == max, so it offers no handle and the dock cannot be resized.
    setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);
}

QSize OutputDock::sizeHint() const
{
    QSize result{ QWidget::sizeHint() };
    result.setWidth(std::max(result.width(), static_cast<int>(NELusanCommon::MIN_OUTPUT_WIDTH)));
    result.setHeight(std::max(result.height(), static_cast<int>(NELusanCommon::MIN_OUTPUT_HEIGHT)));
    return result;
}

QSize OutputDock::minimumSizeHint() const
{
    // A floor low enough that the user can drag the dock down to a thin strip, the way the
    // navigation dock narrows to its own absolute minimum.
    return QSize{ static_cast<int>(NELusanCommon::MIN_OUTPUT_WIDTH), static_cast<int>(NELusanCommon::MIN_OUTPUT_HEIGHT) };
}

void OutputDock::showValidationPlaceholder()
{
    QLabel* placeholder = new QLabel(tr("Open a State Machine document to see its validation findings."), mValidationTab);
    placeholder->setAlignment(Qt::AlignCenter);
    placeholder->setWordWrap(true);
    placeholder->setEnabled(false);
    mValidationView = placeholder;
    mValidation = nullptr;
    mValidationBody->addWidget(mValidationView);
}

SMValidationPanel* OutputDock::ensureValidationPanel()
{
    if (mValidation != nullptr)
    {
        return mValidation;
    }

    delete mValidationView;                 // the placeholder note
    mValidation = new SMValidationPanel(mValidationTab);
    mValidationView = mValidation;
    mValidationBody->addWidget(mValidationView);

    // A finding names the document that owns it, so the reveal goes to that window.
    connect(mValidation, &SMValidationPanel::navigateRequestedIn, this
           , [](QObject* owner, uint32_t elementId, eDocElementKind kind, int rule)
    {
        StateMachine* doc = qobject_cast<StateMachine*>(owner);
        if (doc != nullptr)
        {
            doc->navigateToIssue(elementId, kind, rule);
        }
    });

    connect(mValidation, &SMValidationPanel::pendingCountChanged, this, &OutputDock::updateValidationTitle);
    return mValidation;
}

void OutputDock::updateValidationTitle(int pending)
{
    const int index = mTabs.indexOf(mValidationTab);
    if (index >= 0)
    {
        // The count is the reason to look; with nothing pending the tab must not nag.
        mTabs.setTabText(index, (pending > 0)
                                    ? QStringLiteral("%1 (%2)").arg(OutputDock::TabNameValidation).arg(pending)
                                    : OutputDock::TabNameValidation);
    }
}

void OutputDock::setDocuments(const QList<StateMachine*>& docs)
{
    if (docs.isEmpty())
    {
        // Nothing open: drop the panel so no stale model is held, and show the note again.
        if (mValidation != nullptr)
        {
            delete mValidationView;
            mValidationView = nullptr;
            mValidation = nullptr;
            showValidationPlaceholder();
        }

        mBoundDocs.clear();
        updateValidationTitle(0);
        return;
    }

    SMValidationPanel* panel = ensureValidationPanel();

    // Drop the documents that are gone, then add or refresh the ones that are here. Closing a
    // window must unbind it: the panel holds its facade and would outlive it otherwise.
    for (const QPointer<StateMachine>& bound : mBoundDocs)
    {
        if ((bound.isNull() == false) && (docs.contains(bound.data()) == false))
        {
            panel->removeDocument(bound->getModel());
        }
    }

    mBoundDocs.clear();
    for (StateMachine* doc : docs)
    {
        if (doc == nullptr)
        {
            continue;
        }

        panel->addDocument(doc->getModel(), doc->userFriendlyCurrentFile(), doc);
        mBoundDocs.append(QPointer<StateMachine>(doc));
    }

    updateValidationTitle(panel->pendingCount());
}

void OutputDock::showValidation(int step)
{
    if (mValidationTab != nullptr)
    {
        mTabs.setCurrentWidget(mValidationTab);
    }

    if (mValidation == nullptr)
    {
        return;
    }

    mValidation->refreshNow();
    if (step > 0)
    {
        mValidation->focusNextIssue();
    }
    else if (step < 0)
    {
        mValidation->focusPreviousIssue();
    }
}
