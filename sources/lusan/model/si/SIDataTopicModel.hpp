#ifndef LUSAN_MODEL_SI_SIDATATOPICMODEL_HPP
#define LUSAN_MODEL_SI_SIDATATOPICMODEL_HPP
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
 *  \file        lusan/model/si/SIDataTopicModel.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Data Topic Model.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/si/SIAttributeData.hpp"
#include "lusan/data/si/SIDataTypeData.hpp"

/**
 * \class   SIDataTopicModel
 * \brief   Manages the model for service interface data topics.
 **/
class SIDataTopicModel
{
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Constructor with initialization.
     * \param   attributeData   The instance of SIAttributeData.
     * \param   dataTypeData    The instance of SIDataTypeData.
     **/
    SIDataTopicModel(SIAttributeData& attributeData, SIDataTypeData& dataTypeData);

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Creates an AttributeEntry and sets it in SIAttributeData.
     * \param   name            The name of the attribute.
     * \param   notification    The notification type of the attribute.
     * \return  Valid pointer to the AttributeEntry object. Otherwise, returns nullptr.
     **/
    AttributeEntry * createAttribute(const QString& name, AttributeEntry::eNotification notification = AttributeEntry::eNotification::NotifyOnChange);

    /**
     * \brief   Deletes the attribute by ID.
     * \param   id  The ID of the attribute to delete.
     * \return  True if the attribute was deleted, false otherwise.
     **/
    bool deleteAttribute(uint32_t id);

    /**
     * \brief   Returns the list of attributes.
     **/
    const QList<AttributeEntry>& getAttributes(void) const;

    /**
     * \brief   Searches the attribute entry in the list by given unique ID.
     * \param   id  The unique ID of the attribute element to search.
     * \return  Returns valid pointer if the element found. Otherwise, returns nullptr.
     **/
    const AttributeEntry* findAttribute(uint32_t id) const;
    AttributeEntry* findAttribute(uint32_t id);

    /**
     * \brief   Sorts the attribute elements in the list.
     * \param   ascending   If true, the sorting is ascending. Otherwise, descending.
     **/
    void sortAttributes(bool ascending);

    /**
     * \brief   Replaces the data of attributes in the list of attribute entries.
     * \param   oldDataType     The old data type to replace.
     * \param   newDataType     The new data type to set.
     * \return  Returns the list IDs of attribute entries, which.
     **/
    QList<uint32_t> replaceDataType(DataTypeBase* oldDataType, DataTypeBase* newDataType);

    /**
     * \brief   Returns the instance of data type data object relevant with the attributes.
     **/
    inline SIDataTypeData& getDataTypeData(void);

//////////////////////////////////////////////////////////////////////////
// Hidden member variables.
//////////////////////////////////////////////////////////////////////////
private:
    SIAttributeData&    mData;      //!< Reference to the SIAttributeData instance.
    SIDataTypeData&     mDataType;  //!< Reference to the SIDataTypeData instance.
};

//////////////////////////////////////////////////////////////////////////
// SIDataTopicModel class inline function implementation
//////////////////////////////////////////////////////////////////////////

inline SIDataTypeData& SIDataTopicModel::getDataTypeData(void)
{
    return mDataType;
}

#endif  // LUSAN_MODEL_SI_SIDATATOPICMODEL_HPP
