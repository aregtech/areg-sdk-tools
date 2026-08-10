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
 *  \file        lusan/view/common/OverviewPage.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the Overview page shared by every document editor.
 *
 ************************************************************************/

#include "lusan/view/common/OverviewPage.hpp"

#include "lusan/common/NELusanCommon.hpp"
#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/common/OverviewModel.hpp"
#include "lusan/view/common/PendingEditWatcher.hpp"
#include "lusan/view/common/WidgetHighlight.hpp"

#include <QCheckBox>
#include <QEvent>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSignalBlocker>
#include <QVBoxLayout>

OverviewPage::OverviewPage(OverviewModel& model, const OverviewPageConfig& config, QWidget* parent /*= nullptr*/)
    : QScrollArea       (parent)
    , IEditCommit       ( )
    , mModel            (model)
    , mDetails          (nullptr)
    , mForm             (nullptr)
    , mName             (nullptr)
    , mNameError        (nullptr)
    , mMajor            (nullptr)
    , mMinor            (nullptr)
    , mPatch            (nullptr)
    , mDescription      (nullptr)
    , mDeprecated       (nullptr)
    , mDeprecateHint    (nullptr)
    , mVersionValidator (0, 999999, this)
    , mCommitting       (false)
{
    buildUi(config);
    setupSignals();
    refreshAll();
}

void OverviewPage::revealField(eIssueField field)
{
    switch (field)
    {
    case eIssueField::Name:         WidgetHighlight::reveal(mName);         break;
    case eIssueField::Description:  WidgetHighlight::reveal(mDescription);  break;
    default:                                                                break;
    }
}

void OverviewPage::buildUi(const OverviewPageConfig& config)
{
    QWidget* content = new QWidget(this);
    QVBoxLayout* root = new QVBoxLayout(content);

    QLabel* headline = new QLabel(config.headline, content);
    QFont headlineFont{ headline->font() };
    headlineFont.setPointSize(20);
    headlineFont.setBold(true);
    headlineFont.setItalic(true);
    headline->setFont(headlineFont);
    root->addWidget(headline);

    QHBoxLayout* columns = new QHBoxLayout();

    mDetails = new QGroupBox(tr("Details :"), content);
    mDetails->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    // Everything in this group is document text: typing in it marks the document changed at once,
    // even though the text itself is handed over when the field loses the focus.
    PendingEditWatcher::watchField(mDetails, mModel.getNotifier());
    mForm = new QFormLayout(mDetails);
    mForm->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);
    mForm->setRowWrapPolicy(QFormLayout::DontWrapRows);

    QWidget* nameCell = new QWidget(mDetails);
    QVBoxLayout* nameCellLayout = new QVBoxLayout(nameCell);
    nameCellLayout->setContentsMargins(0, 0, 0, 0);
    nameCellLayout->setSpacing(2);
    mName = new QLineEdit(nameCell);
    mName->setObjectName(QStringLiteral("overviewName"));
    mName->setReadOnly(config.nameEditable == false);
    nameCellLayout->addWidget(mName);
    mNameError = new QLabel(tr("Name must start with a letter or '_' and contain only letters, digits or '_'."), nameCell);
    mNameError->setStyleSheet(QStringLiteral("color: #c0392b;"));
    mNameError->setWordWrap(true);
    mNameError->setVisible(false);
    nameCellLayout->addWidget(mNameError);
    mForm->addRow(tr("Name:"), nameCell);

    QGroupBox* versionBox = new QGroupBox(config.versionTitle, mDetails);
    QGridLayout* versionGrid = new QGridLayout(versionBox);
    mMajor = new QLineEdit(versionBox);
    mMinor = new QLineEdit(versionBox);
    mPatch = new QLineEdit(versionBox);
    mMajor->setObjectName(QStringLiteral("overviewMajor"));
    mMinor->setObjectName(QStringLiteral("overviewMinor"));
    mPatch->setObjectName(QStringLiteral("overviewPatch"));
    const std::initializer_list<QLineEdit*> versionEdits{ mMajor, mMinor, mPatch };
    for (QLineEdit* edit : versionEdits)
    {
        edit->setValidator(&mVersionValidator);
        edit->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        edit->setLayoutDirection(Qt::RightToLeft);
    }

    versionGrid->addWidget(new QLabel(tr("Major"), versionBox), 0, 0);
    versionGrid->addWidget(new QLabel(tr("Minor"), versionBox), 0, 2);
    versionGrid->addWidget(new QLabel(tr("Patch"), versionBox), 0, 4);
    versionGrid->addWidget(mMajor, 1, 0);
    versionGrid->addWidget(new QLabel(QStringLiteral("."), versionBox), 1, 1);
    versionGrid->addWidget(mMinor, 1, 2);
    versionGrid->addWidget(new QLabel(QStringLiteral("."), versionBox), 1, 3);
    versionGrid->addWidget(mPatch, 1, 4);
    mForm->addRow(tr("Version:"), versionBox);

    mDescription = new QPlainTextEdit(mDetails);
    mDescription->setObjectName(QStringLiteral("overviewDescription"));
    mDescription->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    mDescription->setPlaceholderText(config.descriptionHint);
    mDescription->installEventFilter(this);
    mForm->addRow(tr("Description:"), mDescription);

    mDeprecated = new QCheckBox(tr("Deprecated:"), mDetails);
    mDeprecated->setObjectName(QStringLiteral("overviewDeprecated"));
    mDeprecated->setLayoutDirection(Qt::RightToLeft);
    mDeprecateHint = new QLineEdit(mDetails);
    mDeprecateHint->setObjectName(QStringLiteral("overviewDeprecateHint"));
    mDeprecateHint->setEnabled(false);
    mForm->addRow(mDeprecated, mDeprecateHint);

    columns->addWidget(mDetails, 1);

    QWidget* links = new QWidget(content);
    links->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    QVBoxLayout* linksColumn = new QVBoxLayout(links);
    linksColumn->setContentsMargins(0, 0, 0, 0);
    linksColumn->addWidget(buildLinksPanel(config.links));
    linksColumn->addStretch(1);
    columns->addWidget(links, 1);

    root->addLayout(columns, 1);

    // Clamp the content to the viewport width so the two Ignored columns split it 50/50
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setWidgetResizable(true);
    setWidget(content);
}

QWidget* OverviewPage::buildLinksPanel(const QList<OverviewPageLink>& links)
{
    QGroupBox* group = new QGroupBox(tr("Quick Links:"), this);
    QFormLayout* layout = new QFormLayout(group);
    layout->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);

    for (const OverviewPageLink& link : links)
    {
        QPushButton* button = new QPushButton(link.text, group);
        button->setObjectName(link.name);
        button->setFlat(true);
        button->setCursor(Qt::PointingHandCursor);
        button->setToolTip(link.tip);
        QFont font{ button->font() };
        font.setBold(true);
        font.setUnderline(true);
        button->setFont(font);

        const int page = link.page;
        connect(button, &QPushButton::clicked, this, [this, page]() { emit signalPageLinkClicked(page); });

        QLabel* hint = new QLabel(link.hint, group);
        hint->setWordWrap(true);    // keep the panel from forcing the row wider than half the page
        layout->addRow(button, hint);
    }

    return group;
}

void OverviewPage::setupSignals(void)
{
    connect(mName           , &QLineEdit::textEdited        , this, &OverviewPage::onNameEdited);
    connect(mName           , &QLineEdit::editingFinished   , this, &OverviewPage::onNameCommitted);
    connect(mMajor          , &QLineEdit::editingFinished   , this, &OverviewPage::onVersionCommitted);
    connect(mMinor          , &QLineEdit::editingFinished   , this, &OverviewPage::onVersionCommitted);
    connect(mPatch          , &QLineEdit::editingFinished   , this, &OverviewPage::onVersionCommitted);
    connect(mDeprecated     , &QCheckBox::toggled           , this, &OverviewPage::onDeprecatedToggled);
    connect(mDeprecateHint  , &QLineEdit::editingFinished   , this, &OverviewPage::onDeprecateHintCommitted);

    connect(&mModel.getNotifier(), &DocModelNotifier::documentReloaded, this, &OverviewPage::onOverviewChanged);
    connect(&mModel.getNotifier(), &DocModelNotifier::elementChanged, this, [this](uint32_t id, eDocElementKind kind)
    {
        if ((kind == eDocElementKind::Overview) && (id == mModel.getOverviewId()))
        {
            onOverviewChanged();
        }
    });
}

void OverviewPage::refreshAll(void)
{
    const QSignalBlocker blockName(mName);
    const QSignalBlocker blockMajor(mMajor);
    const QSignalBlocker blockMinor(mMinor);
    const QSignalBlocker blockPatch(mPatch);
    const QSignalBlocker blockDescription(mDescription);
    const QSignalBlocker blockDeprecated(mDeprecated);
    const QSignalBlocker blockDeprecateHint(mDeprecateHint);

    mName->setText(mModel.getName());
    showNameValid(NELusanCommon::isValidIdentifier(mModel.getName()));

    const VersionNumber& version = mModel.getVersion();
    mMajor->setText(QString::number(version.getMajor()));
    mMinor->setText(QString::number(version.getMinor()));
    mPatch->setText(QString::number(version.getPatch()));

    mDescription->setPlainText(mModel.getDescription());

    const bool deprecated = mModel.getIsDeprecated();
    mDeprecated->setChecked(deprecated);
    mDeprecateHint->setEnabled(deprecated);
    mDeprecateHint->setText(deprecated ? mModel.getDeprecateHint() : QString());
}

void OverviewPage::onNameEdited(const QString& text)
{
    showNameValid(NELusanCommon::isValidIdentifier(text));
}

void OverviewPage::onNameCommitted(void)
{
    const QString text = mName->text();
    if (mName->isReadOnly() || (NELusanCommon::isValidIdentifier(text) == false))
        return;

    mCommitting = true;
    mModel.setName(text);
    mCommitting = false;
}

void OverviewPage::onVersionCommitted(void)
{
    mCommitting = true;
    mModel.setVersion(VersionNumber(mMajor->text().toUInt(), mMinor->text().toUInt(), mPatch->text().toUInt()));
    mCommitting = false;
}

void OverviewPage::onDeprecatedToggled(bool checked)
{
    mCommitting = true;
    mModel.setIsDeprecated(checked);
    mCommitting = false;

    mDeprecateHint->setEnabled(checked);
    mDeprecateHint->setText(checked ? mModel.getDeprecateHint() : QString());
    if (checked)
    {
        mDeprecateHint->setFocus();
    }
}

void OverviewPage::onDeprecateHintCommitted(void)
{
    if (mDeprecated->isChecked() == false)
        return;

    mCommitting = true;
    mModel.setDeprecateHint(mDeprecateHint->text());
    mCommitting = false;
}

void OverviewPage::onOverviewChanged(void)
{
    if (mCommitting)
        return;

    refreshAll();
}

void OverviewPage::commitDescription(void)
{
    mCommitting = true;
    mModel.setDescription(mDescription->toPlainText());
    mCommitting = false;
}

void OverviewPage::commitPendingEdits(void)
{
    // A save from the keyboard leaves the caret where it is, so no field is asked for its text by
    // losing the focus. Collect them all here, in the order the form shows them.
    onNameCommitted();
    onVersionCommitted();
    onDeprecateHintCommitted();
    commitDescription();
}

bool OverviewPage::eventFilter(QObject* watched, QEvent* event)
{
    if ((watched == mDescription) && (event->type() == QEvent::FocusOut))
    {
        commitDescription();
    }

    return QScrollArea::eventFilter(watched, event);
}

void OverviewPage::showNameValid(bool valid)
{
    mNameError->setVisible(valid == false);
    mName->setStyleSheet(valid ? QString() : QStringLiteral("border: 1px solid #c0392b;"));
}
