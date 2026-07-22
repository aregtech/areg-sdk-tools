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
 *  \file        lusan/view/sm/SMTryStrip.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM guard Try-it strip.
 *
 ************************************************************************/

#include "lusan/view/sm/SMTryStrip.hpp"

#include "lusan/common/NELusanCommon.hpp"
#include "lusan/data/common/DataTypeEnum.hpp"
#include "lusan/data/sm/SMAttributeData.hpp"
#include "lusan/data/sm/SMDataTypeData.hpp"
#include "lusan/data/sm/SMMethodData.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"

#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/sm/SMGuardSymbols.hpp"
#include "lusan/model/sm/SMLiteralValidator.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"

#include "lusan/view/sm/NEGuardStyle.hpp"
#include "lusan/view/sm/SMFlowLayout.hpp"

#include <QComboBox>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPalette>
#include <QStyle>
#include <QTimer>
#include <QToolButton>
#include <QVBoxLayout>

namespace
{
    using eKind  = SMGuardNode::eKind;
    using eTruth = SMGuardEval::eTruth;

    //!< The lens widget-name suffix of a node path (`root`, `0`, `0_2`, ...).
    QString pathName(const QList<int>& path)
    {
        if (path.isEmpty())
        {
            return QStringLiteral("root");
        }

        QStringList parts;
        for (int index : path)
        {
            parts.append(QString::number(index));
        }

        return parts.join(QLatin1Char('_'));
    }

    //!< The enumeration members of a declared type name; empty when not an enumeration.
    QStringList enumMembers(const StateMachineData& data, const QString& typeName)
    {
        QStringList members;
        DataTypeCustom* custom = data.getDataTypes().findCustomDataType(typeName);
        if ((custom != nullptr) && (custom->getCategory() == DataTypeBase::eCategory::Enumeration))
        {
            for (const EnumEntry& field : static_cast<DataTypeEnum*>(custom)->getElements())
            {
                members.append(field.getName());
            }
        }

        return members;
    }
}

//////////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////////

SMTryStrip::SMTryStrip(StateMachineModel& model, QWidget* parent /*= nullptr*/)
    : QWidget           (parent)
    , mModel            (model)
    , mTransitionId     (0u)
    , mToggle           (nullptr)
    , mContent          (nullptr)
    , mResult           (nullptr)
    , mNote             (nullptr)
    , mValues           ( )
    , mStubs            ( )
    , mRebuildPending   (false)
{
    setObjectName(QStringLiteral("smGuardTry"));

    QVBoxLayout* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);
    outer->setSpacing(2);

    mToggle = new QToolButton(this);
    mToggle->setObjectName(QStringLiteral("smGuardTryToggle"));
    mToggle->setText(tr("Try it"));
    mToggle->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    mToggle->setIcon(NELusanCommon::chevronIcon(false, mToggle->palette().color(QPalette::ButtonText), QSize(12, 12)));
    mToggle->setAutoRaise(true);
    mToggle->setCheckable(true);
    mToggle->setChecked(false);
    outer->addWidget(mToggle, 0, Qt::AlignLeft);

    mContent = new QWidget(this);
    mContent->setVisible(false);
    outer->addWidget(mContent);

    connect(mToggle, &QToolButton::toggled, this, [this](bool open)
    {
        mToggle->setIcon(NELusanCommon::chevronIcon(open, mToggle->palette().color(QPalette::ButtonText), QSize(12, 12)));
        mContent->setVisible(open);
        if (open)
        {
            rebuild();
        }
        else
        {
            emit truthTintsChanged(QHash<QString, int>());
        }
    });

    DocModelNotifier& notifier = mModel.getNotifier();
    connect(&notifier, &DocModelNotifier::elementAdded, this, &SMTryStrip::onElementAdded);
    connect(&notifier, &DocModelNotifier::elementChanged, this, &SMTryStrip::onElementChanged);
    connect(&notifier, &DocModelNotifier::elementRemoved, this, &SMTryStrip::onElementRemoved);
    connect(&notifier, &DocModelNotifier::documentReloaded, this, &SMTryStrip::onDocumentReloaded);
}

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////

void SMTryStrip::setTransition(uint32_t transitionId)
{
    if (mTransitionId != transitionId)
    {
        mValues.clear();
        mStubs.clear();
    }

    mTransitionId = transitionId;
    if (isOpen())
    {
        rebuild();
    }
}

bool SMTryStrip::isOpen() const
{
    return mToggle->isChecked();
}

void SMTryStrip::setOpen(bool open)
{
    mToggle->setChecked(open);
}

//////////////////////////////////////////////////////////////////////////
// Model change slots
//////////////////////////////////////////////////////////////////////////

void SMTryStrip::onElementAdded(uint32_t /*id*/, eDocElementKind /*kind*/)
{
    scheduleRebuild();
}

void SMTryStrip::onElementChanged(uint32_t /*id*/, eDocElementKind /*kind*/)
{
    scheduleRebuild();
}

void SMTryStrip::onElementRemoved(uint32_t /*id*/, eDocElementKind /*kind*/)
{
    scheduleRebuild();
}

void SMTryStrip::onDocumentReloaded()
{
    mValues.clear();
    mStubs.clear();
    scheduleRebuild();
}

void SMTryStrip::scheduleRebuild()
{
    if (mRebuildPending || (isOpen() == false))
    {
        return;
    }

    // Deferred: never rebuild inside the emitting command / child signal.
    mRebuildPending = true;
    QTimer::singleShot(0, this, [this]()
    {
        mRebuildPending = false;
        if (isOpen())
        {
            rebuild();
        }
    });
}

//////////////////////////////////////////////////////////////////////////
// Build
//////////////////////////////////////////////////////////////////////////

void SMTryStrip::rebuild()
{
    // Rebuilding replaces the content widget's layout wholesale.
    qDeleteAll(mContent->children());
    mResult = nullptr;
    mNote = nullptr;

    QVBoxLayout* layout = new QVBoxLayout(mContent);
    layout->setContentsMargins(12, 2, 0, 2);
    layout->setSpacing(4);

    const StateMachineData& data = mModel.getData();
    const SMTransitionEntry* transition = (mTransitionId != 0u) ? data.findTransitionById(mTransitionId) : nullptr;
    const SMGuard* guard = (transition != nullptr) ? &transition->getGuard() : nullptr;

    if ((guard == nullptr) || guard->isEmpty() || (guard->isOk() == false) || (guard->getTree() == nullptr))
    {
        mNote = new QLabel(mContent);
        mNote->setObjectName(QStringLiteral("smTryNote"));
        mNote->setWordWrap(true);
        mNote->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
        if ((guard != nullptr) && guard->isDraft())
        {
            // The strip refuses drafts cleanly: nothing to evaluate, no guessing.
            mNote->setText(tr("The guard is a draft -- resolve it first; there is nothing to evaluate."));
        }
        else
        {
            mNote->setText(tr("No guard -- the transition always fires."));
        }

        layout->addWidget(mNote);
        emit truthTintsChanged(QHash<QString, int>());
        return;
    }

    const SMGuardNode& tree = *guard->getTree();

    // Value fields: every stimulus parameter, then every referenced attribute (ours and
    // the siblings' -- the winner line evaluates their guards with the same values).
    QWidget* valueHost = new QWidget(mContent);
    SMFlowLayout* flow = new SMFlowLayout(valueHost, 0, 12, 4);

    const QStringList paramNames = SMGuardSymbols::paramNames(data, mTransitionId);
    const QStringList paramTypes = SMGuardSymbols::paramTypes(data, mTransitionId);
    for (int i = 0; i < paramNames.size(); ++i)
    {
        const uint32_t id = SMGuardSymbols::paramId(data, mTransitionId, paramNames.at(i));
        if (id != 0u)
        {
            flow->addWidget(makeValueField(eKind::Param, id, paramNames.at(i), (i < paramTypes.size()) ? paramTypes.at(i) : QString()));
        }
    }

    QList<uint32_t> attrIds = SMGuardEval::referencedIds(tree, eKind::Attr);
    for (uint32_t siblingId : SMGuardEval::siblingTransitions(data, mTransitionId))
    {
        const SMTransitionEntry* sibling = data.findTransitionById(siblingId);
        if ((sibling != nullptr) && (siblingId != mTransitionId)
            && sibling->getGuard().isOk() && (sibling->getGuard().getTree() != nullptr))
        {
            for (uint32_t id : SMGuardEval::referencedIds(*sibling->getGuard().getTree(), eKind::Attr))
            {
                if (attrIds.contains(id) == false)
                {
                    attrIds.append(id);
                }
            }
        }
    }

    for (uint32_t id : attrIds)
    {
        for (const SMAttributeEntry& attr : data.getAttributes().getElements())
        {
            if (attr.getId() == id)
            {
                flow->addWidget(makeValueField(eKind::Attr, id, attr.getName(), attr.getType()));
                break;
            }
        }
    }

    layout->addWidget(valueHost);

    // Stub rows: calls, islands and raw fragments never execute in the tool.
    const QList<SMGuardEval::StubSite> sites = SMGuardEval::stubSites(tree);
    for (const SMGuardEval::StubSite& site : sites)
    {
        addStubRow(layout, site);
    }

    mResult = new QLabel(mContent);
    mResult->setObjectName(QStringLiteral("smTryResult"));
    mResult->setWordWrap(true);
    mResult->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Preferred);
    layout->addWidget(mResult);

    recompute();
}

QWidget* SMTryStrip::makeValueField(SMGuardNode::eKind kind, uint32_t symbolId, const QString& name, const QString& typeName)
{
    QWidget* row = new QWidget(mContent);
    QHBoxLayout* layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    QLabel* label = new QLabel(QStringLiteral("%1 =").arg(name), row);
    const NEGuardStyle::eOwner owner = (kind == eKind::Param) ? NEGuardStyle::eOwner::Stimulus : NEGuardStyle::eOwner::Fsm;
    label->setStyleSheet(QStringLiteral("color: %1;").arg(NEGuardStyle::ownerColor(owner).name()));
    layout->addWidget(label);

    const QString current = mValues.value(symbolId, defaultValue(kind, symbolId, typeName));
    mValues.insert(symbolId, current);

    const QStringList members = enumMembers(mModel.getData(), typeName);
    const bool isBool = (typeName == QStringLiteral("bool"));
    if (isBool || (members.isEmpty() == false))
    {
        QComboBox* combo = new QComboBox(row);
        combo->setObjectName(QStringLiteral("smTryValue_%1").arg(symbolId));
        combo->addItems(isBool ? QStringList({ QStringLiteral("false"), QStringLiteral("true") }) : members);
        const int index = combo->findText(current);
        combo->setCurrentIndex((index >= 0) ? index : 0);
        mValues.insert(symbolId, combo->currentText());
        connect(combo, &QComboBox::currentTextChanged, this, [this, symbolId](const QString& text)
        {
            mValues.insert(symbolId, text);
            recompute();
        });
        layout->addWidget(combo);
    }
    else
    {
        QLineEdit* edit = new QLineEdit(current, row);
        edit->setObjectName(QStringLiteral("smTryValue_%1").arg(symbolId));
        edit->setMaximumWidth(90);
        connect(edit, &QLineEdit::textChanged, this, [this, symbolId, typeName, edit](const QString& text)
        {
            const QString reason = SMLiteralValidator::validate(typeName, text);
            const bool valid = reason.isEmpty();
            edit->setToolTip(reason);
            edit->setStyleSheet(valid
                                ? QString()
                                : QStringLiteral("border: 1px solid %1;").arg(NEGuardStyle::severityColor(NEGuardStyle::eSeverity::Err).name()));
            // An invalid literal contributes Unknown, never a guess.
            mValues.insert(symbolId, valid ? text : QString());
            recompute();
        });
        layout->addWidget(edit);
    }

    return row;
}

void SMTryStrip::addStubRow(QVBoxLayout* into, const SMGuardEval::StubSite& site)
{
    const StateMachineData& data = mModel.getData();

    QWidget* row = new QWidget(mContent);
    QHBoxLayout* layout = new QHBoxLayout(row);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(4);

    NEGuardStyle::eOwner owner = NEGuardStyle::eOwner::Fsm;
    if (site.kind == eKind::Call)
    {
        const SMMethodEntry* method = SMGuardSymbols::method(data, site.symbolId);
        owner = ((method != nullptr) && method->isLambdaCondition()) ? NEGuardStyle::eOwner::Fsm : NEGuardStyle::eOwner::Handler;
    }
    else if (site.kind == eKind::Raw)
    {
        owner = NEGuardStyle::eOwner::Raw;
    }

    QLabel* glyph = new QLabel(NEGuardStyle::ownerGlyph(owner), row);
    glyph->setStyleSheet(QStringLiteral("color: %1; font-weight: bold;").arg(NEGuardStyle::ownerColor(owner).name()));
    layout->addWidget(glyph);

    // The rendered call with the mapped values substituted (recomputed on value edits).
    SMGuardEval::Inputs inputs;
    inputs.values = mValues;
    const SMTransitionEntry* transition = data.findTransitionById(mTransitionId);
    const SMGuardNode* node = (transition != nullptr) ? transition->getGuard().getTree() : nullptr;
    for (int index : site.path)
    {
        node = (node != nullptr) ? node->childAt(index) : nullptr;
    }

    QLabel* display = new QLabel((node != nullptr) ? SMGuardEval::stubDisplay(data, *node, inputs) : site.text, row);
    display->setObjectName(QStringLiteral("smTryStubText_%1").arg(pathName(site.path)));
    display->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));
    layout->addWidget(display);

    QLabel* arrow = new QLabel(tr("-> stub"), row);
    arrow->setStyleSheet(QStringLiteral("font-style: italic;"));
    layout->addWidget(arrow);

    const QString key = SMGuardEval::pathKey(site.path);
    // Stubs default to true: the strip always shows a decided result to start from.
    const bool current = mStubs.value(key, true);
    mStubs.insert(key, current);

    QComboBox* combo = new QComboBox(row);
    combo->setObjectName(QStringLiteral("smTryStub_%1").arg(pathName(site.path)));
    combo->addItems({ QStringLiteral("true"), QStringLiteral("false") });
    combo->setCurrentIndex(current ? 0 : 1);
    connect(combo, &QComboBox::currentTextChanged, this, [this, key](const QString& text)
    {
        mStubs.insert(key, (text == QStringLiteral("true")));
        recompute();
    });
    layout->addWidget(combo);
    layout->addStretch(1);

    into->addWidget(row);
}

QString SMTryStrip::defaultValue(SMGuardNode::eKind kind, uint32_t symbolId, const QString& typeName) const
{
    SMGuardEval::Inputs none;
    const QString declared = SMGuardEval::effectiveValue(mModel.getData(), kind, symbolId, none);
    if (declared.isEmpty() == false)
    {
        return declared;
    }

    const QStringList members = enumMembers(mModel.getData(), typeName);
    if (members.isEmpty() == false)
    {
        return members.first();
    }

    if (typeName == QStringLiteral("bool"))
    {
        return QStringLiteral("false");
    }

    if (typeName == QStringLiteral("String"))
    {
        return QString();
    }

    return QStringLiteral("0");
}

//////////////////////////////////////////////////////////////////////////
// Evaluation
//////////////////////////////////////////////////////////////////////////

void SMTryStrip::recompute()
{
    if ((mResult == nullptr) || (isOpen() == false))
    {
        return;
    }

    const StateMachineData& data = mModel.getData();
    const SMTransitionEntry* transition = data.findTransitionById(mTransitionId);
    if ((transition == nullptr) || (transition->getGuard().isOk() == false) || (transition->getGuard().getTree() == nullptr))
    {
        return;
    }

    SMGuardEval::Inputs inputs;
    inputs.values = mValues;
    inputs.stubs = mStubs;

    const SMGuardEval::Result result = SMGuardEval::evaluate(data, *transition->getGuard().getTree(), inputs);

    // Refresh the substituted stub-row texts (value edits change them live).
    const QList<SMGuardEval::StubSite> sites = SMGuardEval::stubSites(*transition->getGuard().getTree());
    for (const SMGuardEval::StubSite& site : sites)
    {
        QLabel* display = mContent->findChild<QLabel*>(QStringLiteral("smTryStubText_%1").arg(pathName(site.path)));
        const SMGuardNode* node = transition->getGuard().getTree();
        for (int index : site.path)
        {
            node = (node != nullptr) ? node->childAt(index) : nullptr;
        }

        if ((display != nullptr) && (node != nullptr))
        {
            display->setText(SMGuardEval::stubDisplay(data, *node, inputs));
        }
    }

    QString text;
    switch (result.truth)
    {
    case eTruth::True:
        text = tr("result: guard is TRUE -> transition fires");
        break;
    case eTruth::False:
        text = tr("result: guard is FALSE -> transition does not fire");
        break;
    case eTruth::Unknown:
    default:
        text = tr("result: guard is UNKNOWN -- set the stub values");
        break;
    }

    text += winnerText(result.truth, inputs);
    mResult->setText(text);

    const QColor color = NEGuardStyle::severityColor((result.truth == eTruth::True)
                                                     ? NEGuardStyle::eSeverity::Ok
                                                     : ((result.truth == eTruth::False) ? NEGuardStyle::eSeverity::Err : NEGuardStyle::eSeverity::Warn));
    mResult->setStyleSheet(QStringLiteral("color: %1;").arg(color.name()));

    // Publish the clause truths for the lens tint.
    QHash<QString, int> tints;
    for (const SMGuardEval::NodeTruth& node : result.nodes)
    {
        if (node.truth != eTruth::Unknown)
        {
            tints.insert(pathName(node.path), (node.truth == eTruth::True) ? 1 : 0);
        }
    }

    emit truthTintsChanged(tints);
}

QString SMTryStrip::siblingLabel(uint32_t transitionId) const
{
    const SMTransitionEntry* transition = mModel.getData().findTransitionById(transitionId);
    if (transition == nullptr)
    {
        return QString();
    }

    return transition->isExternal() ? (QStringLiteral("-> ") + transition->getTo()) : tr("(internal)");
}

QString SMTryStrip::winnerText(SMGuardEval::eTruth selfTruth, const SMGuardEval::Inputs& inputs) const
{
    const StateMachineData& data = mModel.getData();
    const QList<uint32_t> siblings = SMGuardEval::siblingTransitions(data, mTransitionId);
    if (siblings.size() <= 1)
    {
        return (selfTruth == eTruth::True) ? tr("; wins over: (none)") : QString();
    }

    // Priority order = document order; sibling guards evaluate with the shared values but
    // without this strip's stubs (their calls stay Unknown -- never a silent guess).
    uint32_t winner = 0u;
    bool undecided = false;
    QStringList wonOver;
    for (uint32_t id : siblings)
    {
        const SMTransitionEntry* sibling = data.findTransitionById(id);
        if (sibling == nullptr)
        {
            continue;
        }

        SMGuardEval::Inputs siblingInputs;
        siblingInputs.values = inputs.values;
        if (id == mTransitionId)
        {
            siblingInputs.stubs = inputs.stubs;
        }

        const eTruth truth = SMGuardEval::guardTruth(data, sibling->getGuard(), siblingInputs);
        if (winner == 0u)
        {
            if (truth == eTruth::True)
            {
                winner = id;
            }
            else if (truth == eTruth::Unknown)
            {
                undecided = true;
                break;
            }
        }
        else if (truth == eTruth::True)
        {
            wonOver.append(siblingLabel(id));
        }
    }

    if (undecided)
    {
        return tr("; priority winner undecided (a sibling guard is unknown)");
    }

    if (winner == mTransitionId)
    {
        return tr("; wins over: %1").arg(wonOver.isEmpty() ? tr("(none)") : wonOver.join(QStringLiteral(", ")));
    }

    if (winner != 0u)
    {
        return tr("; fires instead: %1 (higher priority)").arg(siblingLabel(winner));
    }

    return tr("; no sibling fires");
}
