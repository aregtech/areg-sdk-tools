/************************************************************************
 *  This file is part of the Lusan project, an official component of the AREG SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the AREG Framework.
 *
 *  Lusan is available as free and open-source software under the MIT License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]aregtech.com.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/model/si/SIDataTopicModel.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Data Topic Model.
 *
 ************************************************************************/

 /************************************************************************
  * Includes
  ************************************************************************/
#include "lusan/model/si/SIDataTopicModel.hpp"

  /************************************************************************
   * SIDataTopicModel class implementation
   ************************************************************************/

SIDataTopicModel::SIDataTopicModel(SIAttributeData& attributeData, SIDataTypeData& dataTypeData)
    : mAttributeData(attributeData)
    , mDataTypeData(dataTypeData)
{
}

uint32_t SIDataTopicModel::createAttribute(const QString& name, AttributeEntry::eNotification notification /*= AttributeEntry::eNotification::NotifyOnChange*/)
{
    AttributeEntry newAttribute(mAttributeData.getNextId(), name, "bool", notification);
    mAttributeData.addElement(newAttribute);
    return newAttribute.getId();
}

bool SIDataTopicModel::deleteAttribute(uint32_t id)
{
    return mAttributeData.removeElement(id);
}

const QList<AttributeEntry>& SIDataTopicModel::getAttributes(void) const
{
    return mAttributeData.getElements();
}

const AttributeEntry* SIDataTopicModel::findAttribute(uint32_t id) const
{
    return mAttributeData.findElement(id);
}

AttributeEntry* SIDataTopicModel::findAttribute(uint32_t id)
{
    return mAttributeData.findElement(id);
}

void SIDataTopicModel::sortAttributes(bool ascending)
{
    mAttributeData.sortElementsByName(ascending);
}
