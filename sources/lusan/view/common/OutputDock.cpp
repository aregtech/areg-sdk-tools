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
#include "lusan/view/common/DocValidationPanel.hpp"
#include "lusan/model/common/IEDocumentModel.hpp"
#include "lusan/view/common/MdiChild.hpp"

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

    // The findings of the active state machine document, one per row, worst severity first. A
    // build-style diagnostic list belongs in the output window rather than a per-document panel.
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
    // Expanding vertically on purpose: a Fixed policy makes the content's maximum height equal its
    // hint, which leaves the ADS splitter with min == max and no handle to resize the dock.
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
    QLabel* placeholder = new QLabel(tr("Open a State Machine or a Service Interface document to see its validation findings."), mValidationTab);
    placeholder->setAlignment(Qt::AlignCenter);
    placeholder->setWordWrap(true);
    placeholder->setEnabled(false);
    mValidationView = placeholder;
    mValidation = nullptr;
    mValidationBody->addWidget(mValidationView);
}

DocValidationPanel* OutputDock::ensureValidationPanel()
{
    if (mValidation != nullptr)
    {
        return mValidation;
    }

    delete mValidationView;                 // the placeholder note
    mValidation = new DocValidationPanel(mValidationTab);
    mValidationView = mValidation;
    mValidationBody->addWidget(mValidationView);

    // A finding names the document that owns it, so the reveal goes to that window.
    connect(mValidation, &DocValidationPanel::navigateRequestedIn, this
           , [](QObject* owner, uint32_t elementId, eDocElementKind kind, int rule)
    {
        MdiChild* doc = qobject_cast<MdiChild*>(owner);
        if (doc != nullptr)
        {
            doc->navigateToIssue(elementId, kind, rule);
        }
    });

    connect(mValidation, &DocValidationPanel::pendingCountChanged, this, &OutputDock::updateValidationTitle);
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

void OutputDock::setDocuments(const QList<MdiChild*>& docs)
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

    DocValidationPanel* panel = ensureValidationPanel();

    // Drop the documents that are gone, then add or refresh the ones that are here. Closing a
    // window must unbind it: the panel holds its facade and would outlive it otherwise.
    for (const QPointer<MdiChild>& bound : mBoundDocs)
    {
        IEDocumentModel* model = bound.isNull() ? nullptr : bound->documentModel();
        if ((model != nullptr) && (docs.contains(bound.data()) == false))
        {
            panel->removeDocument(*model);
        }
    }

    mBoundDocs.clear();
    for (MdiChild* doc : docs)
    {
        IEDocumentModel* model = (doc != nullptr) ? doc->documentModel() : nullptr;
        if (model == nullptr)
        {
            continue;
        }

        panel->addDocument(*model, doc->userFriendlyCurrentFile(), doc);
        mBoundDocs.append(QPointer<MdiChild>(doc));
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
