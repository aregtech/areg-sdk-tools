#ifndef LUSAN_MODEL_SI_SICONSTANTMODEL_HPP
#define LUSAN_MODEL_SI_SICONSTANTMODEL_HPP
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
 *  \file        lusan/model/si/SIConstantModel.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Constant Model.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/data/si/SIConstantData.hpp"
#include "lusan/data/si/SIDataTypeData.hpp"

/**
 * \class   SIConstantModel
 * \brief   Manages the model for service interface constants.
 **/
class SIConstantModel
{
//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Constructor with initialization.
     * \param   constantData    The instance of SIConstantData.
     * \param   dataTypeData    The instance of SIDataTypeData.
     **/
    SIConstantModel(SIConstantData& constantData, SIDataTypeData& dataTypeData);

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Creates a ConstantEntry and sets it in SIConstantData.
     * \param   name            The name of the constant.
     * \return  Valid pointer to the new created constant element. Otherwise, returns nullptr.
     **/
    ConstantEntry* createConstant(const QString& name);

    /**
     * \brief   Deletes the constant by ID.
     * \param   id  The ID of the constant to delete.
     * \return  True if the constant was deleted, false otherwise.
     **/
    bool deleteConstant(uint32_t id);

    /**
     * \brief   Returns the list of constants.
     **/
    const QList<ConstantEntry> & getConstants(void) const;

    /**
     * \brief   Searches the constant entry in the list by given unique ID.
     * \param   id  The unique ID of the constant element to search.
     * \return  Returns valid pointer if the element found. Otherwise, returns nullptr.
     **/
    const ConstantEntry* findConstant(uint32_t id) const;
    ConstantEntry* findConstant(uint32_t id);

    /**
     * \brief   Sorts the constant elements in the list..
     * \param   ascending   If true, the sorting is ascending. Otherwise, descending.
     **/
    void sortConstants(bool ascending);

    /**
     * \brief   Replaces the data of constants in the list of constant entries.
     * \param   oldDataType     The old data type to replace.
     * \param   newDataType     The new data type to set.
     * \return  Returns the list IDs of constant entries, which.
     **/
    QList<uint32_t> replaceDataType(DataTypeBase* oldDataType, DataTypeBase* newDataType);

    /**
     * \brief   Swaps the constants by given unique IDs.
     *          The swapping will not change the order of IDs, but will swap the data.
     * \param   firstId     The unique ID of the first constant to swap.
     * \param   secondId    The unique ID of the second constant to swap.
     **/
    void swapConstants(uint32_t firstId, uint32_t secondId);

    /**
     * \brief   Swaps the constants by given constant entries.
     *          The swapping will not change the order of IDs, but will swap the data.
     * \param   first       The first constant entry to swap.
     * \param   second      The second constant entry to swap.
     **/
    void swapConstants(const ConstantEntry& first, const ConstantEntry& second);

    /**
     * \brief   Returns the instance of data type data object relevant with the constants.
     **/
    inline SIDataTypeData& getDataTypeData(void);

//////////////////////////////////////////////////////////////////////////
// Hidden member variables.
//////////////////////////////////////////////////////////////////////////
private:
    SIConstantData& mData;      //!< Reference to the SIConstantData instance.
    SIDataTypeData& mDataType;  //!< Reference to the SIDataTypeData instance.
};

//////////////////////////////////////////////////////////////////////////
// SIConstantModel class inline function implementation
//////////////////////////////////////////////////////////////////////////

inline SIDataTypeData& SIConstantModel::getDataTypeData(void)
{
    return mDataType;
}

#endif  // LUSAN_MODEL_SI_SICONSTANTMODEL_HPP
