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
 *  \file        lusan/model/common/DocModelNotifier.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, per-document change-notification hub shared by editors.
 *
 ************************************************************************/

#include "lusan/model/common/DocModelNotifier.hpp"

DocModelNotifier::DocModelNotifier(QObject* parent)
    : QObject(parent)
{
}

void DocModelNotifier::notifyElementAdded(uint32_t id, eDocElementKind kind)
{
    emit elementAdded(id, kind);
}

void DocModelNotifier::notifyElementRemoved(uint32_t id, eDocElementKind kind)
{
    emit elementRemoved(id, kind);
}

void DocModelNotifier::notifyElementChanged(uint32_t id, eDocElementKind kind)
{
    emit elementChanged(id, kind);
}

void DocModelNotifier::notifyListReordered(uint32_t ownerId, eDocElementKind kind)
{
    emit listReordered(ownerId, kind);
}

void DocModelNotifier::notifyLayoutChanged(const QList<uint32_t>& ownerIds)
{
    emit layoutChanged(ownerIds);
}

void DocModelNotifier::notifyNameChanged(uint32_t id, const QString& oldName, const QString& newName)
{
    emit nameChanged(id, oldName, newName);
}

void DocModelNotifier::notifyDocumentReloaded(void)
{
    emit documentReloaded();
}

void DocModelNotifier::notifyReadOnlyChanged(bool readOnly)
{
    emit readOnlyChanged(readOnly);
}
