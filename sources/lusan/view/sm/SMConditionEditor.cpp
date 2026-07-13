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
 *  \file        lusan/view/sm/SMConditionEditor.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM transition condition builder (Conditions tab).
 *
 ************************************************************************/

#include "lusan/view/sm/SMConditionEditor.hpp"

#include "lusan/data/common/ConstantEntry.hpp"
#include "lusan/data/common/MethodBase.hpp"
#include "lusan/data/common/MethodParameter.hpp"
#include "lusan/data/sm/SMAttributeData.hpp"
#include "lusan/data/sm/SMConstantData.hpp"
#include "lusan/data/sm/SMEventData.hpp"
#include "lusan/data/sm/SMMethodData.hpp"
#include "lusan/data/sm/SMTimerData.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"

#include "lusan/model/common/DocModelNotifier.hpp"
#include "lusan/model/sm/SMConditionCommands.hpp"
#include "lusan/model/sm/SMConditionText.hpp"
#include "lusan/model/sm/SMConditionToken.hpp"
#include "lusan/model/sm/SMLiteralValidator.hpp"
#include "lusan/model/sm/SMTypeCompat.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"

#include "lusan/view/sm/SMCodeEditor.hpp"

#include <QCheckBox>
#include <QComboBox>
#include <QEvent>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QStringList>
#include <QTimer>
#include <QVBoxLayout>

//////////////////////////////////////////////////////////////////////////
// Per-row widget bundle
//////////////////////////////////////////////////////////////////////////

struct SMConditionEditor::LeafRow
{
    uint32_t            leafId  { 0u };
    bool                verbatim{ false };
    int                 vkind   { 0 };       //!< eValueSource for a verbatim row (Expression/Lambda).
    QCheckBox*          neg     { nullptr };
    QComboBox*          lhs     { nullptr };
    QComboBox*          op      { nullptr };
    QComboBox*          rhs     { nullptr };
    QPlainTextEdit*     code    { nullptr }; //!< The verbatim body editor (verbatim rows).
    QList<QLineEdit*>   args;                //!< cond:: argument-mapping fields.
    QString             condName;            //!< The cond:: method whose args the grid maps.
    QLabel*             feedback{ nullptr };
};

namespace
{
    using eSource = SMArgumentEntry::eValueSource;
    using eOper   = SMConditionEntry::eOperator;

    //!< Deletes every widget/sub-layout under \p layout, leaving it empty.
    void clearLayout(QLayout* layout)
    {
        if (layout == nullptr)
        {
            return;
        }

        while (QLayoutItem* item = layout->takeAt(0))
        {
            if (QWidget* w = item->widget())
            {
                w->deleteLater();
            }

            if (QLayout* child = item->layout())
            {
                clearLayout(child);
            }

            delete item;
        }
    }

    //!< The `prefix::name` token for an operand kind + reference (picker text / arg prefill).
    QString operandToken(int kind, const QString& name)
    {
        switch (static_cast<eSource>(kind))
        {
        case eSource::Param:        return QStringLiteral("arg::")   + name;
        case eSource::Attribute:    return QStringLiteral("attr::")  + name;
        case eSource::Constant:     return QStringLiteral("const::") + name;
        case eSource::Condition:    return QStringLiteral("cond::")  + name;
        case eSource::Value:        return QStringLiteral("val::")   + name;
        default:                    return name;
        }
    }

    //!< The C++ symbol for a relational operator (empty for a bool-test row).
    QString opSymbol(eOper op)
    {
        switch (op)
        {
        case eOper::Equal:          return QStringLiteral("==");
        case eOper::NotEqual:       return QStringLiteral("!=");
        case eOper::Less:           return QStringLiteral("<");
        case eOper::LessEqual:      return QStringLiteral("<=");
        case eOper::Greater:        return QStringLiteral(">");
        case eOper::GreaterEqual:   return QStringLiteral(">=");
        default:                    return QString();
        }
    }

    //!< True when a token opens a verbatim block (`expr::{` / `lambda::{`), which is a whole row.
    bool isVerbatimToken(const QString& token)
    {
        return token.startsWith(QStringLiteral("expr::{")) || token.startsWith(QStringLiteral("lambda::{"));
    }
}

//////////////////////////////////////////////////////////////////////////
// Construction
//////////////////////////////////////////////////////////////////////////

SMConditionEditor::SMConditionEditor(StateMachineModel& model, QWidget* parent /*= nullptr*/)
    : QWidget         (parent)
    , mModel          (model)
    , mTransitionId   (0u)
    , mTreeLayout     (nullptr)
    , mPreview        (nullptr)
    , mRebuildPending (false)
{
    QVBoxLayout* outer = new QVBoxLayout(this);
    outer->setContentsMargins(0, 0, 0, 0);

    QScrollArea* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    QWidget* host = new QWidget(scroll);
    mTreeLayout = new QVBoxLayout(host);
    mTreeLayout->setAlignment(Qt::AlignTop);
    scroll->setWidget(host);
    outer->addWidget(scroll, 1);

    QLabel* previewCaption = new QLabel(tr("Preview:"), this);
    outer->addWidget(previewCaption);
    mPreview = new QLabel(this);
    mPreview->setWordWrap(true);
    mPreview->setTextInteractionFlags(Qt::TextSelectableByMouse);
    mPreview->setStyleSheet(QStringLiteral("font-family: monospace;"));
    outer->addWidget(mPreview);

    DocModelNotifier& notifier = mModel.getNotifier();
    connect(&notifier, &DocModelNotifier::elementChanged, this, &SMConditionEditor::onElementChanged);
    connect(&notifier, &DocModelNotifier::elementRemoved, this, &SMConditionEditor::onElementRemoved);
    connect(&notifier, &DocModelNotifier::documentReloaded, this, &SMConditionEditor::onDocumentReloaded);

    rebuild();
}

SMConditionEditor::~SMConditionEditor()
{
    mModel.getNotifier().disconnect(this);
}

//////////////////////////////////////////////////////////////////////////
// Public
//////////////////////////////////////////////////////////////////////////

void SMConditionEditor::setTransition(uint32_t transitionId)
{
    mTransitionId = transitionId;
    rebuild();
}

SMConditionGroup* SMConditionEditor::rootGroup() const
{
    if (mTransitionId == 0u)
    {
        return nullptr;
    }

    SMTransitionEntry* transition = mModel.getData().findTransitionById(mTransitionId);
    return (transition != nullptr) ? &transition->getConditions() : nullptr;
}

//////////////////////////////////////////////////////////////////////////
// Notifications
//////////////////////////////////////////////////////////////////////////

void SMConditionEditor::onElementChanged(uint32_t id, eDocElementKind /*kind*/)
{
    if (id == mTransitionId)
    {
        scheduleRebuild();
    }
}

void SMConditionEditor::onElementRemoved(uint32_t id, eDocElementKind /*kind*/)
{
    if (id == mTransitionId)
    {
        mTransitionId = 0u;
        scheduleRebuild();
    }
}

void SMConditionEditor::onDocumentReloaded()
{
    mTransitionId = 0u;
    scheduleRebuild();
}

void SMConditionEditor::scheduleRebuild()
{
    if (mRebuildPending)
    {
        return;
    }

    // Never rebuild synchronously inside a notifier slot: the command that emitted it is still
    // on the stack and a widget it lives under may be mid-event. Defer to the next turn.
    mRebuildPending = true;
    QTimer::singleShot(0, this, [this]()
    {
        mRebuildPending = false;
        rebuild();
    });
}

//////////////////////////////////////////////////////////////////////////
// Build
//////////////////////////////////////////////////////////////////////////

void SMConditionEditor::rebuild()
{
    mCodeRows.clear();
    clearLayout(mTreeLayout);

    SMConditionGroup* root = rootGroup();
    if (root == nullptr)
    {
        mPreview->clear();
        QLabel* hint = new QLabel(tr("Select a transition to edit its conditions."), this);
        hint->setEnabled(false);
        mTreeLayout->addWidget(hint);
        return;
    }

    buildGroup(mTreeLayout, *root, nullptr, 0);
    mPreview->setText(root->isEmpty() ? tr("(always true)") : SMConditionText::preview(*root));
}

void SMConditionEditor::addReorderButtons(QHBoxLayout* line, uint32_t parentGroupId, int index, int count)
{
    QPushButton* up = new QPushButton(QStringLiteral("^"), this);
    QPushButton* down = new QPushButton(QStringLiteral("v"), this);
    up->setEnabled(index > 0);
    down->setEnabled(index < (count - 1));
    up->setMaximumWidth(24);
    down->setMaximumWidth(24);
    line->addWidget(up);
    line->addWidget(down);

    connect(up, &QPushButton::clicked, this, [this, parentGroupId, index]()
    {
        mModel.getUndoStack().push(new SMReorderConditionCommand(mModel.getData(), mModel.getNotifier(), mTransitionId, parentGroupId, index, index - 1, tr("Reorder condition")));
    });
    connect(down, &QPushButton::clicked, this, [this, parentGroupId, index]()
    {
        mModel.getUndoStack().push(new SMReorderConditionCommand(mModel.getData(), mModel.getNotifier(), mTransitionId, parentGroupId, index, index + 1, tr("Reorder condition")));
    });
}

void SMConditionEditor::buildGroup(QVBoxLayout* parentLayout, SMConditionGroup& group, SMConditionGroup* parent, int depth)
{
    const bool isRoot = (parent == nullptr);
    const uint32_t groupId = group.getId();

    QFrame* box = new QFrame(this);
    box->setFrameShape(QFrame::StyledPanel);
    QVBoxLayout* inner = new QVBoxLayout(box);
    inner->setContentsMargins(depth == 0 ? 4 : 12, 4, 4, 4);

    // Header: combine + negate + add/remove.
    QHBoxLayout* header = new QHBoxLayout();
    header->addWidget(new QLabel(isRoot ? tr("Match:") : tr("Group:"), box));

    QComboBox* combine = new QComboBox(box);
    combine->addItem(tr("ALL (AND)"), static_cast<int>(SMConditionGroup::eCombine::And));
    combine->addItem(tr("ANY (OR)"),  static_cast<int>(SMConditionGroup::eCombine::Or));
    combine->setCurrentIndex(group.getCombine() == SMConditionGroup::eCombine::Or ? 1 : 0);
    header->addWidget(combine);

    QCheckBox* negate = new QCheckBox(QStringLiteral("!"), box);
    negate->setToolTip(tr("Negate the whole group"));
    negate->setChecked(group.isNegated());
    header->addWidget(negate);

    header->addStretch(1);

    QPushButton* addRule = new QPushButton(tr("+ Rule"), box);
    QPushButton* addGroup = new QPushButton(tr("+ Group"), box);
    header->addWidget(addRule);
    header->addWidget(addGroup);

    if (isRoot == false)
    {
        addReorderButtons(header, parent->getId(), parent->indexOfChild(&group), parent->getCount());

        QPushButton* removeGroup = new QPushButton(tr("-"), box);
        removeGroup->setToolTip(tr("Remove this group"));
        header->addWidget(removeGroup);
        connect(removeGroup, &QPushButton::clicked, this, [this, groupId]()
        {
            mModel.getUndoStack().push(new SMRemoveConditionCommand(mModel.getData(), mModel.getNotifier(), mTransitionId, groupId, tr("Remove condition group")));
        });
    }

    inner->addLayout(header);

    connect(combine, QOverload<int>::of(&QComboBox::activated), this, [this, groupId, combine](int)
    {
        const SMConditionGroup::eCombine value = static_cast<SMConditionGroup::eCombine>(combine->currentData().toInt());
        mModel.getUndoStack().push(new SMSetGroupCombineCommand(mModel.getData(), mModel.getNotifier(), mTransitionId, groupId, value, tr("Set group combine")));
    });
    connect(negate, &QCheckBox::toggled, this, [this, groupId](bool on)
    {
        mModel.getUndoStack().push(new SMSetGroupNegateCommand(mModel.getData(), mModel.getNotifier(), mTransitionId, groupId, on, tr("Negate group")));
    });
    connect(addRule, &QPushButton::clicked, this, [this, groupId]()
    {
        mModel.getUndoStack().push(new SMAddConditionCommand(mModel.getData(), mModel.getNotifier(), mTransitionId, groupId, false, tr("Add condition")));
    });
    connect(addGroup, &QPushButton::clicked, this, [this, groupId]()
    {
        mModel.getUndoStack().push(new SMAddConditionCommand(mModel.getData(), mModel.getNotifier(), mTransitionId, groupId, true, tr("Add condition group")));
    });

    for (SMConditionNode* child : group.getChildren())
    {
        if (child->isLeaf())
        {
            buildLeaf(inner, *static_cast<SMConditionEntry*>(child), group);
        }
        else
        {
            buildGroup(inner, *static_cast<SMConditionGroup*>(child), &group, depth + 1);
        }
    }

    parentLayout->addWidget(box);
}

void SMConditionEditor::buildLeaf(QVBoxLayout* parentLayout, SMConditionEntry& leaf, SMConditionGroup& group)
{
    std::shared_ptr<LeafRow> row = std::make_shared<LeafRow>();
    row->leafId = leaf.getId();
    const uint32_t leafId = row->leafId;

    QFrame* cell = new QFrame(this);
    QVBoxLayout* cellLayout = new QVBoxLayout(cell);
    cellLayout->setContentsMargins(0, 0, 0, 0);

    QHBoxLayout* line = new QHBoxLayout();

    row->neg = new QCheckBox(QStringLiteral("!"), cell);
    row->neg->setToolTip(tr("Negate this rule"));
    row->neg->setChecked(leaf.isNegated());
    line->addWidget(row->neg);

    if (leaf.isVerbatimRow())
    {
        row->verbatim = true;
        row->vkind = static_cast<int>(leaf.getLhsKind());

        SMCodeEditor* code = new SMCodeEditor(cell);
        code->setSignature(leaf.isLambdaRow() ? tr("lambda:: boolean body (must return bool)") : tr("expr:: boolean expression"));
        code->setNote(tr("Verbatim C++; the machine instance is captured. Never parsed."));
        code->ctrlBody()->setPlainText(leaf.isLambdaRow() ? leaf.getBody() : leaf.getExpression());
        row->code = code->ctrlBody();
        row->code->installEventFilter(this);
        mCodeRows.insert(row->code, row);
        line->addWidget(code, 1);

        if (leaf.isLambdaRow())
        {
            QPushButton* promote = new QPushButton(tr("Promote to method"), cell);
            promote->setToolTip(tr("Move this lambda body to a reusable named Condition method"));
            line->addWidget(promote);
            connect(promote, &QPushButton::clicked, this, [this, leafId]()
            {
                StateMachineData& data = mModel.getData();
                QString name = QStringLiteral("Condition");
                for (int i = 1; data.getMethods().findMethod(name) != nullptr; ++i)
                {
                    name = QStringLiteral("Condition") + QString::number(i);
                }

                mModel.getUndoStack().push(new SMPromoteLambdaCommand(data, mModel.getNotifier(), mTransitionId, leafId, name, tr("Promote lambda to method")));
            });
        }
    }
    else
    {
        row->lhs = new QComboBox(cell);
        row->lhs->setEditable(true);
        row->lhs->setInsertPolicy(QComboBox::NoInsert);
        fillOperandPicker(row->lhs, true);
        row->lhs->setEditText(operandToken(static_cast<int>(leaf.getLhsKind()), leaf.getLhs()));
        line->addWidget(row->lhs, 1);

        row->op = new QComboBox(cell);
        row->op->addItem(tr("(bool)"),  static_cast<int>(eOper::None));
        row->op->addItem(QStringLiteral("=="), static_cast<int>(eOper::Equal));
        row->op->addItem(QStringLiteral("!="), static_cast<int>(eOper::NotEqual));
        row->op->addItem(QStringLiteral("<"),  static_cast<int>(eOper::Less));
        row->op->addItem(QStringLiteral("<="), static_cast<int>(eOper::LessEqual));
        row->op->addItem(QStringLiteral(">"),  static_cast<int>(eOper::Greater));
        row->op->addItem(QStringLiteral(">="), static_cast<int>(eOper::GreaterEqual));
        row->op->setCurrentIndex(row->op->findData(static_cast<int>(leaf.getOperator())));
        line->addWidget(row->op);

        row->rhs = new QComboBox(cell);
        row->rhs->setEditable(true);
        row->rhs->setInsertPolicy(QComboBox::NoInsert);
        fillOperandPicker(row->rhs, false);
        row->rhs->setEditText(operandToken(static_cast<int>(leaf.getRhsKind()), leaf.getRhs()));
        row->rhs->setVisible(leaf.hasOperator());
        line->addWidget(row->rhs, 1);
    }

    addReorderButtons(line, group.getId(), group.indexOfChild(&leaf), group.getCount());

    QPushButton* remove = new QPushButton(QStringLiteral("x"), cell);
    remove->setToolTip(tr("Remove this rule"));
    line->addWidget(remove);
    connect(remove, &QPushButton::clicked, this, [this, leafId]()
    {
        mModel.getUndoStack().push(new SMRemoveConditionCommand(mModel.getData(), mModel.getNotifier(), mTransitionId, leafId, tr("Remove condition")));
    });

    cellLayout->addLayout(line);

    // Condition-method argument mapping grid (design 5.4, minimal).
    if ((row->verbatim == false) && (leaf.getLhsKind() == eSource::Condition))
    {
        SMMethodEntry* method = mModel.getData().getMethods().findMethod(leaf.getLhs());
        if ((method != nullptr) && (method->getElementCount() > 0))
        {
            row->condName = leaf.getLhs();
            const QList<SMArgumentEntry>& stored = leaf.getArguments();
            int index = 0;
            for (const MethodParameter& param : method->getElements())
            {
                QHBoxLayout* argLine = new QHBoxLayout();
                argLine->addSpacing(20);
                argLine->addWidget(new QLabel(param.getName() + QStringLiteral(" : ") + param.getType(), cell));

                QLineEdit* field = new QLineEdit(cell);
                if (index < stored.size())
                {
                    field->setText(operandToken(static_cast<int>(stored.at(index).getSource()), stored.at(index).getValue()));
                }
                argLine->addWidget(field, 1);
                row->args.append(field);
                cellLayout->addLayout(argLine);
                ++index;
            }
        }
    }

    row->feedback = new QLabel(cell);
    row->feedback->setWordWrap(true);
    row->feedback->setStyleSheet(QStringLiteral("color: #c0392b;"));
    cellLayout->addWidget(row->feedback);

    // Wire commits: every editable control funnels through commitLeaf().
    connect(row->neg, &QCheckBox::toggled, this, [this, row]() { commitLeaf(row); });
    if (row->lhs != nullptr)
    {
        connect(row->lhs, QOverload<int>::of(&QComboBox::activated), this, [this, row](int) { commitLeaf(row); });
        connect(row->lhs->lineEdit(), &QLineEdit::editingFinished, this, [this, row]() { commitLeaf(row); });
    }
    if (row->op != nullptr)
    {
        connect(row->op, QOverload<int>::of(&QComboBox::activated), this, [this, row](int) { commitLeaf(row); });
    }
    if (row->rhs != nullptr)
    {
        connect(row->rhs, QOverload<int>::of(&QComboBox::activated), this, [this, row](int) { commitLeaf(row); });
        connect(row->rhs->lineEdit(), &QLineEdit::editingFinished, this, [this, row]() { commitLeaf(row); });
    }
    for (QLineEdit* field : row->args)
    {
        connect(field, &QLineEdit::editingFinished, this, [this, row]() { commitLeaf(row); });
    }

    updateFeedback(row);
    parentLayout->addWidget(cell);
}

//////////////////////////////////////////////////////////////////////////
// Commit
//////////////////////////////////////////////////////////////////////////

void SMConditionEditor::commitLeaf(const std::shared_ptr<LeafRow>& row)
{
    SMConditionGroup* root = rootGroup();
    SMConditionEntry* leaf = (root != nullptr) ? root->findLeaf(row->leafId) : nullptr;
    if (leaf == nullptr)
    {
        return;
    }

    const QString neg = row->neg->isChecked() ? QStringLiteral("!") : QString();
    SMConditionEntry content;
    QString error;

    if (row->verbatim)
    {
        content.setNegated(row->neg->isChecked());
        content.setLhsKind(static_cast<eSource>(row->vkind));
        const QString body = (row->code != nullptr) ? row->code->toPlainText() : QString();
        if (static_cast<eSource>(row->vkind) == eSource::Lambda)
        {
            content.setBody(body);
        }
        else
        {
            content.setExpression(body);
        }
    }
    else
    {
        QString lhsTok = row->lhs->currentText().trimmed();

        // A cond:: with a live argument grid assembles its call from the grid fields.
        if ((row->args.isEmpty() == false) && (lhsTok == QStringLiteral("cond::") + row->condName))
        {
            QStringList parts;
            for (QLineEdit* field : row->args)
            {
                const QString token = field->text().trimmed();
                if (token.isEmpty() == false)
                {
                    parts << token;
                }
            }

            lhsTok = QStringLiteral("cond::") + row->condName + QChar('(') + parts.join(QStringLiteral(", ")) + QChar(')');
        }

        QString line;
        if (isVerbatimToken(lhsTok))
        {
            line = neg + lhsTok;   // verbatim block is a whole row; ignore op/rhs
        }
        else
        {
            const eOper op = static_cast<eOper>(row->op->currentData().toInt());
            if (op == eOper::None)
            {
                line = neg + lhsTok;
            }
            else
            {
                line = neg + lhsTok + QChar(' ') + opSymbol(op) + QChar(' ') + row->rhs->currentText().trimmed();
            }
        }

        if (SMConditionToken::parseLeaf(line, content, error) == false)
        {
            row->feedback->setText(error);   // non-blocking: show the reason, do not commit
            return;
        }
    }

    // Skip a no-op edit so it does not create an empty undo step.
    if (SMConditionToken::renderLeaf(content) == SMConditionToken::renderLeaf(*leaf))
    {
        updateFeedback(row);
        return;
    }

    mModel.getUndoStack().push(new SMSetConditionLeafCommand(mModel.getData(), mModel.getNotifier(), mTransitionId, row->leafId, content, tr("Edit condition")));
}

bool SMConditionEditor::eventFilter(QObject* watched, QEvent* event)
{
    if (event->type() == QEvent::FocusOut)
    {
        const auto it = mCodeRows.constFind(watched);
        if (it != mCodeRows.constEnd())
        {
            commitLeaf(it.value());
        }
    }

    return QWidget::eventFilter(watched, event);
}

//////////////////////////////////////////////////////////////////////////
// Pickers and feedback
//////////////////////////////////////////////////////////////////////////

void SMConditionEditor::fillOperandPicker(QComboBox* combo, bool includeVerbatim) const
{
    StateMachineData& data = mModel.getData();
    combo->clear();

    for (const SMAttributeEntry& attr : data.getAttributes().getElements())
    {
        combo->addItem(QStringLiteral("attr::") + attr.getName());
    }

    // arg:: operands come from the stimulus payload (disabled for a Timer stimulus, spec 6.7).
    const SMTransitionEntry* transition = data.findTransitionById(mTransitionId);
    if ((transition != nullptr) && (transition->getStimulusKind() != SMTransitionEntry::eStimulusKind::Timer))
    {
        const MethodBase* payload = nullptr;
        if (transition->getStimulusKind() == SMTransitionEntry::eStimulusKind::Trigger)
        {
            payload = data.getMethods().findMethod(transition->getStimulus());
        }
        else
        {
            for (const SMEventEntry* event : data.getEvents().getElements())
            {
                if ((event != nullptr) && (event->getName() == transition->getStimulus()))
                {
                    payload = event;
                    break;
                }
            }
        }

        if (payload != nullptr)
        {
            for (const MethodParameter& param : payload->getElements())
            {
                combo->addItem(QStringLiteral("arg::") + param.getName());
            }
        }
    }

    for (const ConstantEntry& constant : data.getConstants().getElements())
    {
        combo->addItem(QStringLiteral("const::") + constant.getName());
    }

    for (const SMMethodEntry* method : data.getMethods().getElements())
    {
        if ((method != nullptr) && method->isCondition())
        {
            combo->addItem(QStringLiteral("cond::") + method->getName());
        }
    }

    combo->addItem(QStringLiteral("val::"));

    if (includeVerbatim)
    {
        combo->addItem(QStringLiteral("expr::{ }"));
        combo->addItem(QStringLiteral("lambda::{ }"));
    }
}

QString SMConditionEditor::operandType(int kind, const QString& ref) const
{
    StateMachineData& data = mModel.getData();
    switch (static_cast<eSource>(kind))
    {
    case eSource::Attribute:
    {
        const SMAttributeEntry* attr = data.getAttributes().findElement(ref);
        return (attr != nullptr) ? attr->getType() : QString();
    }

    case eSource::Constant:
    {
        const ConstantEntry* constant = data.getConstants().findElement(ref);
        return (constant != nullptr) ? constant->getType() : QString();
    }

    case eSource::Param:
    {
        const SMTransitionEntry* transition = data.findTransitionById(mTransitionId);
        const MethodBase* payload = nullptr;
        if (transition != nullptr)
        {
            if (transition->getStimulusKind() == SMTransitionEntry::eStimulusKind::Trigger)
            {
                payload = data.getMethods().findMethod(transition->getStimulus());
            }
            else if (transition->getStimulusKind() == SMTransitionEntry::eStimulusKind::Event)
            {
                for (const SMEventEntry* event : data.getEvents().getElements())
                {
                    if ((event != nullptr) && (event->getName() == transition->getStimulus()))
                    {
                        payload = event;
                        break;
                    }
                }
            }
        }

        if (payload != nullptr)
        {
            for (const MethodParameter& param : payload->getElements())
            {
                if (param.getName() == ref)
                {
                    return param.getType();
                }
            }
        }

        return QString();
    }

    case eSource::Condition:
        return QStringLiteral("bool");

    default:
        return QString();
    }
}

void SMConditionEditor::updateFeedback(const std::shared_ptr<LeafRow>& row)
{
    if (row->feedback == nullptr)
    {
        return;
    }

    SMConditionGroup* root = rootGroup();
    SMConditionEntry* leaf = (root != nullptr) ? root->findLeaf(row->leafId) : nullptr;
    if ((leaf == nullptr) || leaf->isVerbatimRow() || (leaf->hasOperator() == false))
    {
        row->feedback->clear();
        return;
    }

    const QString lt = operandType(static_cast<int>(leaf->getLhsKind()), leaf->getLhs());
    const QString rt = operandType(static_cast<int>(leaf->getRhsKind()), leaf->getRhs());

    QString reason = SMTypeCompat::areComparable(lt, leaf->getOperator(), rt);
    if (reason.isEmpty())
    {
        // Literal syntax check: a val:: side against the other side's declared type.
        if (leaf->getRhsKind() == eSource::Value)
        {
            reason = SMLiteralValidator::validate(lt, leaf->getRhs());
        }
        else if (leaf->getLhsKind() == eSource::Value)
        {
            reason = SMLiteralValidator::validate(rt, leaf->getLhs());
        }
    }

    row->feedback->setText(reason);
}
