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
     * \param   type            The data type name of the constant.
     * \param   value           The value of the constant.
     * \param   isDeprecated    The deprecated flag of the constant.
     * \param   description     The description of the constant.
     * \param   deprecateHint   The deprecation hint of the constant.
     * \return  True if the constant was added, false otherwise.
     **/
    bool createConstant(const QString& name, const QString& type, const QString& value, bool isDeprecated, const QString& description, const QString& deprecateHint);

    /**
     * \brief   Deletes a ConstantEntry in SIConstantData by name.
     * \param   name    The name of the constant to delete.
     * \return  True if the constant was deleted, false otherwise.
     **/
    bool deleteConstant(const QString& name);

    /**
     * \brief   Updates the value of a ConstantEntry.
     * \param   name    The name of the constant to update.
     * \param   value   The new value of the constant.
     * \return  True if the constant was updated, false otherwise.
     **/
    bool updateConstantValue(const QString& name, const QString& value);

    /**
     * \brief   Updates the data type of a ConstantEntry.
     * \param   name    The name of the constant to update.
     * \param   type    The new data type of the constant.
     * \return  True if the constant was updated, false otherwise.
     **/
    bool updateConstantType(const QString& name, const QString& type);

    bool addContant(const ConstantEntry& newEntry, bool unique = true);
    
    bool updateConstantName(const QString& oldName, const QString& newName);

    bool updateConstantDeprecation(const QString& name, bool isDeprecated);

    bool updateConstantDescription(const QString& name, const QString& description);

    bool updateConstantDeprecateHint(const QString& name, const QString& deprecateHint);

    bool updateConstant(const QString& name, const QString& type, const QString& value, bool isDeprecated, const QString& description, const QString& deprecateHint);

    void getConstantTypes(QStringList& out_dataTypes) const;

    const QList<ConstantEntry> & getConstants(void) const;

    const ConstantEntry* findConstant(const QString& name) const;

    inline SIDataTypeData& getDataTypeData(void);

//////////////////////////////////////////////////////////////////////////
// Hidden member variables.
//////////////////////////////////////////////////////////////////////////
private:
    SIConstantData& mConstantData; //!< Reference to the SIConstantData instance.
    SIDataTypeData& mDataTypeData; //!< Reference to the SIDataTypeData instance.
};

//////////////////////////////////////////////////////////////////////////
// SIConstantModel class inline function implementation
//////////////////////////////////////////////////////////////////////////

inline SIDataTypeData& SIConstantModel::getDataTypeData(void)
{
    return mDataTypeData;
}

#endif  // LUSAN_MODEL_SI_SICONSTANTMODEL_HPP
