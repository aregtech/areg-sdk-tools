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
 *  \file        lusan/model/si/SIIncludeModel.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Includes Model.
 *
 ************************************************************************/
#ifndef LUSAN_MODEL_SI_SIINCLUDEMODEL_HPP
#define LUSAN_MODEL_SI_SIINCLUDEMODEL_HPP

/************************************************************************
 * Includes
 ************************************************************************/
#include <QAbstractTableModel>
#include "lusan/data/si/SIIncludeData.hpp"

/************************************************************************
 * Dependencies
 ************************************************************************/
class SIIncludeData;

/**
 * \class   SIIncludeModel
 * \brief   Model for managing include entries in a table view.
 **/
class SIIncludeModel
{
//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Constructor with initialization.
     * \param   includeData    Reference to the SIIncludeData instance.
     **/
    explicit SIIncludeModel(SIIncludeData& includeData);

//////////////////////////////////////////////////////////////////////////
// Operations and attributes
//////////////////////////////////////////////////////////////////////////
public:
    
    /**
     * \brief   Creates a IncludeEntry and sets it in SIIncludeData.
     * \param   name            The name of the constant.
     * \return  True if the constant was added, false otherwise.
     **/
    uint32_t createInclude(const QString& name);

    /**
     * \brief   Deletes the constant by ID.
     * \param   id  The ID of the constant to delete.
     * \return  True if the constant was deleted, false otherwise.
     **/
    bool deleteInclude(uint32_t id);

    /**
     * \brief   Returns the list of include objects.
     **/
    const QList<IncludeEntry>& getIncludes(void) const;

    /**
     * \brief   Searches the include entry in the list by given unique ID.
     * \param   id  The unique ID of the include element to search.
     * \return  Returns valid pointer if the element found. Otherwise, returns nullptr.
     **/
    const IncludeEntry* findInclude(uint32_t id) const;
    IncludeEntry* findInclude(uint32_t id);

    /**
     * \brief   Sorts the include elements in the list.
     * \param   ascending   If true, sorts in ascending order. Otherwise, sorts in descending order.
     **/
    void sortInclude(bool ascending);

//////////////////////////////////////////////////////////////////////////
// hidden members
//////////////////////////////////////////////////////////////////////////
private:
    SIIncludeData& mIncludeData;  //!< Reference to the SIIncludeData instance.
};

#endif // LUSAN_MODEL_SI_SIINCLUDEMODEL_HPP
