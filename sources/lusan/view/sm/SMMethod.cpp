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
 *  \file        lusan/view/sm/SMMethod.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the state machine's Methods page.
 *
 ************************************************************************/

#include "lusan/view/sm/SMMethod.hpp"

#include "lusan/data/common/MethodDataSection.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/common/MethodModel.hpp"
#include "lusan/model/sm/SMGuardCodegenPreview.hpp"
#include "lusan/model/sm/SMGuardWhereUsed.hpp"
#include "lusan/model/sm/SMSelectionModel.hpp"
#include "lusan/model/sm/SMSymbolIndex.hpp"
#include "lusan/model/sm/SMWhereUsed.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/view/common/MethodDetailsView.hpp"
#include "lusan/view/common/MethodListView.hpp"
#include "lusan/view/sm/SMCodeEditor.hpp"

#include <QAction>
#include <QCursor>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QTreeWidget>
#include <QTreeWidgetItem>

namespace
{
    const MethodPageConfig& pageConfig(void)
    {
        static const MethodPageConfig _config
        {
              QObject::tr("State Machine Methods Editor ...")
            , QObject::tr("Methods:")
            , QObject::tr("Type:")
            , true
        };

        return _config;
    }
}

SMMethod::SMMethod(MethodModel& model, StateMachineModel& facade, QWidget* parent /*= nullptr*/)
    : MethodPage(model, pageConfig(), parent)
    , mFacade   (facade)
{
    // The guard-use line navigates to the guard that uses the condition.
    connect(getDetails()->ctrlGuardInfo(), &QLabel::linkActivated, this, [this](const QString& link)
    {
        static const QString _prefix{ QStringLiteral("uses:") };
        if (link.startsWith(_prefix))
        {
            showMethodWhereUsed(link.mid(_prefix.length()).toUInt());
        }
    });

    QTreeWidget* table = getList()->ctrlTableList();
    table->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(table, &QTreeWidget::customContextMenuRequested, this, [this, table](const QPoint& pos)
    {
        QTreeWidgetItem* item = table->itemAt(pos);
        if (item == nullptr)
            return;

        table->setCurrentItem(item);
        MethodEntry* method = currentMethod();
        if ((method == nullptr) || (method->hasImplement() == false))
            return;

        QMenu menu(this);
        QAction* editBody = method->isEmbedded() ? menu.addAction(tr("Edit body")) : nullptr;
        QAction* whereUsed = menu.addAction(tr("Where used"));
        QAction* chosen = menu.exec(table->viewport()->mapToGlobal(pos));
        if ((editBody != nullptr) && (chosen == editBody))
        {
            getDetails()->ctrlBody()->ctrlBody()->setFocus();
        }
        else if (chosen == whereUsed)
        {
            showMethodWhereUsed(method->getId());
        }
    });

    // The base page built its tree while this object was still a MethodPage, so none of the
    // rows carry the machine's own marks yet. Build it again, now that they are reachable.
    refreshAll();
}

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////

void SMMethod::whereUsedForCurrent(void)
{
    MethodEntry* method = currentMethod();
    if (method == nullptr)
    {
        QMessageBox::information(this, tr("Where used"), tr("Select a method first."));
        return;
    }

    showMethodWhereUsed(method->getId());
}

bool SMMethod::currentReference(SMReferences::eTarget& target, uint32_t& id, QString& name) const
{
    MethodEntry* method = currentMethod();
    if (method == nullptr)
        return false;

    target = targetOf(*method);
    id     = method->getId();
    name   = method->getName();
    return true;
}

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////

bool SMMethod::confirmRemove(uint32_t id)
{
    MethodEntry* method = getModel().findMethod(id);
    if (method == nullptr)
        return false;

    // A condition a guard still names cannot go: the guard trees bind it by ID and would dangle.
    if (method->hasImplement())
    {
        const QList<SMGuardWhereUsed::Use> uses = SMGuardWhereUsed::symbolUses(mFacade.getData(), id);
        if (uses.isEmpty() == false)
        {
            QStringList places;
            for (const SMGuardWhereUsed::Use& use : uses)
            {
                places.append(QStringLiteral("  - ") + use.location);
            }

            QMessageBox::warning(this, tr("Cannot delete '%1'").arg(method->getName())
                                , tr("'%1' is used by %2 guard%3:\n%4\n\nRemove it from those guards first.")
                                  .arg(method->getName())
                                  .arg(uses.size())
                                  .arg((uses.size() == 1) ? QString() : QStringLiteral("s"))
                                  .arg(places.join(QLatin1Char('\n'))));
            return false;
        }
    }

    // Anything else that names it by name is a warning, not a refusal: the references become
    // unresolved-reference findings, which the author may well want.
    const QList<SMReferences::Use> refs = SMWhereUsed::collect(mFacade.getData(), targetOf(*method), method->getName(), id);
    if (refs.isEmpty())
        return true;

    QStringList places;
    for (const SMReferences::Use& use : refs)
    {
        places.append(QStringLiteral("  - ") + use.location);
    }

    const QMessageBox::StandardButton answer = QMessageBox::question(this
        , tr("Delete '%1'?").arg(method->getName())
        , tr("'%1' is used in %2 place%3:\n%4\n\nDelete it anyway? The references become unresolved-reference errors.")
          .arg(method->getName())
          .arg(refs.size())
          .arg((refs.size() == 1) ? QString() : QStringLiteral("s"))
          .arg(places.join(QLatin1Char('\n')))
        , QMessageBox::Yes | QMessageBox::No, QMessageBox::No);

    return (answer == QMessageBox::Yes);
}

QString SMMethod::nameCollisionReason(const MethodEntry* method, const QString& name, uint32_t selfId) const
{
    const QString reason = MethodPage::nameCollisionReason(method, name, selfId);
    if ((reason.isEmpty() == false) || (method == nullptr) || name.isEmpty())
        return reason;

    // Only a trigger takes part in the machine-wide stimulus name space.
    if (method->getKind() != NEMethod::SmTrigger)
        return QString();

    const StateMachineData::StimulusRef ref = mFacade.getData().findStimulus(name);
    if ((ref.element == nullptr) || (ref.element->getId() == selfId))
        return QString();

    if (ref.type == StateMachineData::eStimulusType::Event)
        return tr("'%1' is already used by an event").arg(name);
    if (ref.type == StateMachineData::eStimulusType::Timer)
        return tr("'%1' is already used by a timer").arg(name);

    return QString();
}

void SMMethod::decorateMethodNode(QTreeWidgetItem* node, const MethodEntry& method) const
{
    if (method.hasImplement() == false)
        return;

    // The type column carries the implementation mode as a glyph and a word.
    const bool embedded = method.isEmbedded();
    node->setText(static_cast<int>(MethodListView::ColType), embedded ? tr("{} lambda") : tr("h handler"));
    node->setToolTip(static_cast<int>(MethodListView::ColType), embedded
                     ? tr("condition with its body written in Lusan, generated as a std::function member")
                     : tr("condition implemented by your handler"));
}

void SMMethod::updateExtraFields(MethodEntry* method)
{
    const bool showGuardInfo = (method != nullptr) && method->hasImplement();
    if (showGuardInfo)
    {
        const bool embedded = method->isEmbedded();
        const int uses = SMGuardWhereUsed::useCount(mFacade.getData(), method->getId());
        const QString call = embedded
                             ? QString::fromLatin1(SMGuardCodegenPreview::LAMBDA_MEMBER_PREFIX) + method->getName() + QStringLiteral("(...)")
                             : QString::fromLatin1(SMGuardCodegenPreview::HANDLER_ACCESSOR) + QLatin1Char('.') + method->getName() + QStringLiteral("(...)");
        getDetails()->ctrlGuardInfo()->setText(tr("%1, called as <tt>%2</tt>, <a href=\"uses:%3\">used by %4 guard%5</a>")
                                               .arg(embedded ? QStringLiteral("{} lambda") : QStringLiteral("h handler"))
                                               .arg(call.toHtmlEscaped())
                                               .arg(method->getId())
                                               .arg(uses)
                                               .arg((uses == 1) ? QString() : QStringLiteral("s")));
    }

    getDetails()->setGuardInfoVisible(showGuardInfo);
}

void SMMethod::updateBodyEditor(MethodEntry* method)
{
    MethodPage::updateBodyEditor(method);
    if ((method == nullptr) || (method->hasImplement() == false))
        return;

    getDetails()->ctrlBody()->setNote(tr("The machine instance is always captured: attributes, constants and accessors are in scope. "
                                         "The body must end in 'return <value>;' of the declared type."));
    // An embedded condition body is not transition-scoped: no stimulus parameters in scope.
    getDetails()->ctrlBody()->setCompletions(SMSymbolIndex::completionWords(mFacade.getData(), 0, false));
}

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////

SMReferences::eTarget SMMethod::targetOf(const MethodEntry& method) const
{
    switch (method.getKind())
    {
    case NEMethod::SmAction:    return SMReferences::eTarget::Action;
    case NEMethod::SmCondition: return SMReferences::eTarget::Condition;
    default:                    return SMReferences::eTarget::Trigger;
    }
}

void SMMethod::showMethodWhereUsed(uint32_t methodId)
{
    MethodEntry* method = getModel().findMethod(methodId);
    if (method == nullptr)
        return;

    // Every reference kind of the method: transition stimuli, action calls and condition
    // mappings from the shared walker, plus the ID-bound guard uses of a condition.
    const QList<SMReferences::Use> uses = SMWhereUsed::collect(mFacade.getData(), targetOf(*method), method->getName(), methodId);
    if (uses.isEmpty())
    {
        QMessageBox::information(this, tr("Where used"), tr("'%1' is not referenced anywhere.").arg(method->getName()));
        return;
    }

    QMenu menu(this);
    for (const SMReferences::Use& use : uses)
    {
        QAction* action = menu.addAction(use.location);
        const uint32_t navId = use.navId;
        connect(action, &QAction::triggered, this, [this, navId]()
        {
            mFacade.getSelectionModel().setSelection({ navId });
        });
    }

    menu.exec(QCursor::pos());
}
