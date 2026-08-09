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
 *  \file        lusan/view/sm/SMConstant.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Constants page.
 *
 ************************************************************************/

#include "lusan/view/sm/SMConstant.hpp"

#include "lusan/data/common/ConstantEntry.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/sm/SMWhereUsed.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/view/sm/SMWhereUsedMenu.hpp"

#include <QMessageBox>
#include <QStringList>

SMConstant::SMConstant(ConstantModel& model, StateMachineModel& facade, QWidget* parent /*= nullptr*/)
    : ConstantPage  (model, tr("State Machine Constant Editor ..."), parent)
    , mFacade       (facade)
{
}

void SMConstant::whereUsedForCurrent(void)
{
    const uint32_t id = currentConstantId();
    if (id == 0)
    {
        QMessageBox::information(this, tr("Where used"), tr("Select a constant first."));
        return;
    }

    StateMachineData& data = mFacade.getData();
    const ConstantEntry* constant = data.getConstants().findElement(id);
    const QString name = (constant != nullptr) ? constant->getName() : QString();
    const QList<SMReferences::Use> uses = SMWhereUsed::collect(data, SMReferences::eTarget::Constant, name, id);
    SMWhereUsedMenu::present(this, uses, mFacade.getSelectionModel(), name);
}

bool SMConstant::currentReference(SMReferences::eTarget& target, uint32_t& id, QString& name) const
{
    const uint32_t current = currentConstantId();
    if (current == 0)
        return false;

    const ConstantEntry* constant = mFacade.getData().getConstants().findElement(current);
    if (constant == nullptr)
        return false;

    target = SMReferences::eTarget::Constant;
    id   = current;
    name = constant->getName();
    return true;
}

bool SMConstant::confirmRemove(uint32_t id)
{
    StateMachineData& data = mFacade.getData();
    const ConstantEntry* constant = data.getConstants().findElement(id);
    const QString name = (constant != nullptr) ? constant->getName() : QString();
    const QList<SMReferences::Use> uses = SMWhereUsed::collect(data, SMReferences::eTarget::Constant, name, id);
    if (uses.isEmpty())
    {
        return true;
    }

    QStringList places;
    for (const SMReferences::Use& use : uses)
    {
        places.append(QStringLiteral("  - ") + use.location);
    }

    const QMessageBox::StandardButton choice = QMessageBox::warning(this, tr("Constant is referenced")
                        , tr("This constant is used in %1 place%2:\n%3\n\nDelete anyway? The references break and are listed by validation.")
                          .arg(uses.size())
                          .arg((uses.size() == 1) ? QString() : QStringLiteral("s"))
                          .arg(places.join(QLatin1Char('\n')))
                        , QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);

    return (choice == QMessageBox::Yes);
}
