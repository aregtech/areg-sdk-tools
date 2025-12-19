/************************************************************************
 *  This file is part of the Lusan project, an official component of the AREG SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the AREG Framework.
 *
 *  Lusan is available as free and open-source software under the MIT License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]areg.tech.
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
     * \param   location    The file path included in service interface.
     * \return  Valid pointer to the new include entry object. Otherwise, returns nullptr.
     **/
    IncludeEntry * createInclude(const QString& location);

    /**
     * \brief   Inserts the include entry in the list of include entries.
     * \param   position    The position to insert the include entry.
     * \param   location    The file path included in service interface.
     * \return  Valid pointer to the new created include element. Otherwise, returns nullptr.
     **/
    IncludeEntry* insertInclude(int position, const QString& location);

    /**
     * \brief   Deletes the include by ID.
     * \param   id  The ID of the include to delete.
     * \return  True if the include was deleted, false otherwise.
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

    /**
     * \brief   Swaps the includes by given unique IDs.
     *          The swapping will not change the order of IDs, but will swap the data.
     * \param   firstId     The unique ID of the first include to swap.
     * \param   secondId    The unique ID of the second include to swap.
     **/
    void swapIncludes(uint32_t firstId, uint32_t secondId);

    /**
     * \brief   Swaps the includes by given include entries.
     *          The swapping will not change the order of IDs, but will swap the data.
     * \param   first       The first include entry to swap.
     * \param   second      The second include entry to swap.
     **/
    void swapIncludes(const IncludeEntry& first, const IncludeEntry& second);

//////////////////////////////////////////////////////////////////////////
// hidden members
//////////////////////////////////////////////////////////////////////////
private:
    SIIncludeData& mData;   //!< Reference to the SIIncludeData instance.
};

#endif // LUSAN_MODEL_SI_SIINCLUDEMODEL_HPP
