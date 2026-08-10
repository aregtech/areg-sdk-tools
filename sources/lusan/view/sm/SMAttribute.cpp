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
 *  \file        lusan/view/sm/SMAttribute.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Attributes page.
 *
 ************************************************************************/

#include "lusan/view/sm/SMAttribute.hpp"

#include "lusan/data/common/AttributeEntry.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/sm/SMWhereUsed.hpp"
#include "lusan/model/sm/StateMachineModel.hpp"
#include "lusan/view/sm/SMWhereUsedMenu.hpp"

#include <QMessageBox>
#include <QStringList>

SMAttribute::SMAttribute(AttributeModel& model, StateMachineModel& facade, QWidget* parent /*= nullptr*/)
    : AttributePage (model, tr("State Machine Attribute Editor ..."), parent)
    , mFacade       (facade)
{
}

void SMAttribute::whereUsedForCurrent(void)
{
    const uint32_t id = currentAttributeId();
    if (id == 0)
    {
        QMessageBox::information(this, tr("Where used"), tr("Select an attribute first."));
        return;
    }

    StateMachineData& data = mFacade.getData();
    const AttributeEntry* attribute = data.getAttributes().findElement(id);
    const QString name = (attribute != nullptr) ? attribute->getName() : QString();
    const QList<SMReferences::Use> uses = SMWhereUsed::collect(data, SMReferences::eTarget::Attribute, name, id);
    SMWhereUsedMenu::present(this, uses, mFacade.getSelectionModel(), name);
}

bool SMAttribute::currentReference(SMReferences::eTarget& target, uint32_t& id, QString& name) const
{
    const uint32_t current = currentAttributeId();
    if (current == 0)
        return false;

    const AttributeEntry* attribute = mFacade.getData().getAttributes().findElement(current);
    if (attribute == nullptr)
        return false;

    target = SMReferences::eTarget::Attribute;
    id   = current;
    name = attribute->getName();
    return true;
}

bool SMAttribute::confirmRemove(uint32_t id)
{
    StateMachineData& data = mFacade.getData();
    const AttributeEntry* attribute = data.getAttributes().findElement(id);
    const QString name = (attribute != nullptr) ? attribute->getName() : QString();
    const QList<SMReferences::Use> uses = SMWhereUsed::collect(data, SMReferences::eTarget::Attribute, name, id);
    if (uses.isEmpty())
    {
        return true;
    }

    QStringList places;
    for (const SMReferences::Use& use : uses)
    {
        places.append(QStringLiteral("  - ") + use.location);
    }

    const QMessageBox::StandardButton choice = QMessageBox::warning(this, tr("Attribute is referenced")
                        , tr("This attribute is used in %1 place%2:\n%3\n\nDelete anyway? The references break and are listed by validation.")
                          .arg(uses.size())
                          .arg((uses.size() == 1) ? QString() : QStringLiteral("s"))
                          .arg(places.join(QLatin1Char('\n')))
                        , QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);

    return (choice == QMessageBox::Yes);
}
