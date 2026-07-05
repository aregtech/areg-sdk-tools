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
 *  \copyright   © 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/view/sm/SMOverview.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Overview page.
 *
 ************************************************************************/

#include "lusan/view/sm/SMOverview.hpp"

#include "lusan/model/sm/SMOverviewModel.hpp"
#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/view/sm/StateMachine.hpp"

#include <QEvent>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QRegularExpression>
#include <QSignalBlocker>
#include <QVBoxLayout>

SMOverview::SMOverview(SMOverviewModel& model, QWidget* parent /*= nullptr*/)
    : QScrollArea       (parent)
    , mModel            (model)
    , mName             (nullptr)
    , mNameError        (nullptr)
    , mMajor            (nullptr)
    , mMinor            (nullptr)
    , mPatch            (nullptr)
    , mShared           (nullptr)
    , mLocal            (nullptr)
    , mDescription      (nullptr)
    , mVersionValidator (0, 999999, this)
    , mCommitting       (false)
{
    buildUi();
    setupSignals();
    updateData();
}

void SMOverview::buildUi(void)
{
    QWidget* content = new QWidget(this);
    QHBoxLayout* columns = new QHBoxLayout(content);

    QWidget* details = new QWidget(content);
    QVBoxLayout* outer = new QVBoxLayout(details);
    outer->setContentsMargins(0, 0, 0, 0);

    QFormLayout* form = new QFormLayout();
    mName = new QLineEdit(content);
    form->addRow(tr("Machine name:"), mName);

    mNameError = new QLabel(tr("Name must start with a letter or '_' and contain only letters, digits or '_'."), content);
    mNameError->setStyleSheet(QStringLiteral("color: #c0392b;"));
    mNameError->setWordWrap(true);
    mNameError->setVisible(false);
    form->addRow(QString(), mNameError);

    QWidget* versionBox = new QWidget(content);
    QHBoxLayout* versionLayout = new QHBoxLayout(versionBox);
    versionLayout->setContentsMargins(0, 0, 0, 0);
    mMajor = new QLineEdit(versionBox);
    mMinor = new QLineEdit(versionBox);
    mPatch = new QLineEdit(versionBox);
    mMajor->setValidator(&mVersionValidator);
    mMinor->setValidator(&mVersionValidator);
    mPatch->setValidator(&mVersionValidator);
    versionLayout->addWidget(mMajor);
    versionLayout->addWidget(new QLabel(QStringLiteral("."), versionBox));
    versionLayout->addWidget(mMinor);
    versionLayout->addWidget(new QLabel(QStringLiteral("."), versionBox));
    versionLayout->addWidget(mPatch);
    form->addRow(tr("Version:"), versionBox);

    outer->addLayout(form);

    QGroupBox* threading = new QGroupBox(tr("Threading mode"), content);
    QVBoxLayout* threadingLayout = new QVBoxLayout(threading);
    mShared = new QRadioButton(tr("Shared"), threading);
    QLabel* sharedHint = new QLabel(tr("The machine may be driven from more than one thread; generated code is thread-safe."), threading);
    sharedHint->setWordWrap(true);
    mLocal = new QRadioButton(tr("Local"), threading);
    QLabel* localHint = new QLabel(tr("The whole machine is guaranteed to live in a single thread; generated code has no locking."), threading);
    localHint->setWordWrap(true);
    threadingLayout->addWidget(mShared);
    threadingLayout->addWidget(sharedHint);
    threadingLayout->addWidget(mLocal);
    threadingLayout->addWidget(localHint);
    outer->addWidget(threading);

    outer->addWidget(new QLabel(tr("Description:"), details));
    mDescription = new QPlainTextEdit(details);
    mDescription->installEventFilter(this);
    outer->addWidget(mDescription, 1);

    columns->addWidget(details, 1);
    columns->addWidget(buildLinksPanel(), 0, Qt::AlignTop);

    setWidgetResizable(true);
    setWidget(content);
}

QWidget* SMOverview::buildLinksPanel(void)
{
    struct Link
    {
        int             page;
        const char*     text;
        const char*     tip;
        const char*     hint;
    };

    static const Link _links[] =
    {
          { StateMachine::PageDataTypes , QT_TR_NOOP("Data Types ...") , QT_TR_NOOP("Click to open the Data Types page") , QT_TR_NOOP("Declare enumerations, structures and imported types") }
        , { StateMachine::PageAttributes, QT_TR_NOOP("Attributes ...") , QT_TR_NOOP("Click to open the Attributes page")  , QT_TR_NOOP("Declare the machine's internal data variables")     }
        , { StateMachine::PageEvents    , QT_TR_NOOP("Events ...")     , QT_TR_NOOP("Click to open the Events page")      , QT_TR_NOOP("Declare events and timers")                         }
        , { StateMachine::PageMethods   , QT_TR_NOOP("Methods ...")    , QT_TR_NOOP("Click to open the Methods page")     , QT_TR_NOOP("Declare triggers, actions and conditions")          }
        , { StateMachine::PageConstants , QT_TR_NOOP("Constants ...")  , QT_TR_NOOP("Click to open the Constants page")   , QT_TR_NOOP("Declare named typed literals")                      }
        , { StateMachine::PageIncludes  , QT_TR_NOOP("Includes ...")   , QT_TR_NOOP("Click to open the Includes page")    , QT_TR_NOOP("Declare include files and imported machines")       }
        , { StateMachine::PageDesign    , QT_TR_NOOP("Design ...")     , QT_TR_NOOP("Click to open the Design page")      , QT_TR_NOOP("Edit the state graph on the canvas")                }
    };

    QGroupBox* group = new QGroupBox(tr("Quick Links:"), this);
    QFormLayout* layout = new QFormLayout(group);
    layout->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);

    for (const Link& link : _links)
    {
        QPushButton* button = new QPushButton(tr(link.text), group);
        button->setFlat(true);
        button->setCursor(Qt::PointingHandCursor);
        button->setToolTip(tr(link.tip));
        QFont font{ button->font() };
        font.setBold(true);
        font.setUnderline(true);
        button->setFont(font);

        const int page = link.page;
        connect(button, &QPushButton::clicked, this, [this, page]() { emit signalPageLinkClicked(page); });

        layout->addRow(button, new QLabel(tr(link.hint), group));
    }

    return group;
}

void SMOverview::setupSignals(void)
{
    connect(mName , &QLineEdit::textEdited     , this, &SMOverview::onNameEdited);
    connect(mName , &QLineEdit::editingFinished, this, &SMOverview::onNameCommitted);
    connect(mMajor, &QLineEdit::textEdited      , this, &SMOverview::onVersionEdited);
    connect(mMinor, &QLineEdit::textEdited      , this, &SMOverview::onVersionEdited);
    connect(mPatch, &QLineEdit::textEdited      , this, &SMOverview::onVersionEdited);
    connect(mShared, &QRadioButton::toggled     , this, &SMOverview::onThreadingToggled);

    connect(&mModel.getNotifier(), &DocModelNotifier::documentReloaded, this, &SMOverview::onOverviewChanged);
    connect(&mModel.getNotifier(), &DocModelNotifier::elementChanged, this, [this](uint32_t id, eDocElementKind kind)
    {
        if ((kind == eDocElementKind::Overview) && (id == mModel.getOverviewId()))
        {
            onOverviewChanged();
        }
    });
}

void SMOverview::updateData(void)
{
    const QSignalBlocker blockName(mName);
    const QSignalBlocker blockMajor(mMajor);
    const QSignalBlocker blockMinor(mMinor);
    const QSignalBlocker blockPatch(mPatch);
    const QSignalBlocker blockShared(mShared);
    const QSignalBlocker blockLocal(mLocal);
    const QSignalBlocker blockDescription(mDescription);

    mName->setText(mModel.getName());
    showNameValid(isValidMachineName(mModel.getName()));

    const VersionNumber& version = mModel.getVersion();
    mMajor->setText(QString::number(version.getMajor()));
    mMinor->setText(QString::number(version.getMinor()));
    mPatch->setText(QString::number(version.getPatch()));

    if (mModel.getThreading() == SMOverviewData::eThreading::Local)
        mLocal->setChecked(true);
    else
        mShared->setChecked(true);

    mDescription->setPlainText(mModel.getDescription());
}

void SMOverview::onNameEdited(const QString& text)
{
    showNameValid(isValidMachineName(text));
}

void SMOverview::onNameCommitted(void)
{
    const QString text = mName->text();
    if (isValidMachineName(text))
    {
        mCommitting = true;
        mModel.setName(text);
        mCommitting = false;
    }
}

void SMOverview::onVersionEdited(void)
{
    mCommitting = true;
    mModel.setVersion(VersionNumber(mMajor->text().toUInt(), mMinor->text().toUInt(), mPatch->text().toUInt()));
    mCommitting = false;
}

void SMOverview::onThreadingToggled(bool checked)
{
    mCommitting = true;
    mModel.setThreading(checked ? SMOverviewData::eThreading::Shared : SMOverviewData::eThreading::Local);
    mCommitting = false;
}

void SMOverview::onOverviewChanged(void)
{
    if (mCommitting)
        return;

    updateData();
}

void SMOverview::commitDescription(void)
{
    mCommitting = true;
    mModel.setDescription(mDescription->toPlainText());
    mCommitting = false;
}

bool SMOverview::eventFilter(QObject* watched, QEvent* event)
{
    if ((watched == mDescription) && (event->type() == QEvent::FocusOut))
    {
        commitDescription();
    }

    return QScrollArea::eventFilter(watched, event);
}

void SMOverview::showNameValid(bool valid)
{
    mNameError->setVisible(valid == false);
    mName->setStyleSheet(valid ? QString() : QStringLiteral("border: 1px solid #c0392b;"));
}

bool SMOverview::isValidMachineName(const QString& name)
{
    static const QRegularExpression _identifier{ QStringLiteral("^[A-Za-z_][A-Za-z0-9_]*$") };
    return _identifier.match(name).hasMatch();
}
