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
     * \brief   Updates the model object.
     **/
    void updateModel(void);

    /**
     * \brief   Updates the data object.
     **/
    void updateData(void);

    /**
     * \brief   Returns the number of rows in the model.
     **/
    int rowCount(void) const;

    /**
     * \brief   Returns the number of columns in the model.
     * \param   parent  The parent index.
     * \return  The number of columns.
     **/
    int columnCount(void) const;

    /**
     * \brief   Returns the data stored under the given row index.
     * \param   row     The index of the item.
     * \return  The data for the item or nullptr if there is no data under the given index.
     **/
    const IncludeEntry * data(int row) const;

    /**
     * \brief   Returns the data stored under the given unique location.
     * \param   location    The unique location of the item.
     * \return  The data for the item or nullptr if there is no data with the specified location.
     **/
    const IncludeEntry * data(const QString & location) const;

    /**
     * \brief   Adds a new unique entry.
     * \param   location        The file path.
     * \param   description     The description.
     * \param   isDeprecated    The deprecated flag.
     * \param   deprecateHint   The deprecation hint.
     * \return  Returns true if new entry with the unique location is added. Otherwise, returns false.
     **/
    bool addEntry(const QString& location, const QString& description, bool isDeprecated, const QString& deprecateHint);

    /**
     * \brief   Updates an entry by index.
     * \param   index           The index of the entry to update.
     * \param   location        The new location.
     * \param   description     The new description.
     * \param   isDeprecated    The new deprecated flag.
     * \param   deprecateHint   The new deprecation hint.
     * \return  True if the entry was updated, false otherwise.
     **/
    bool updateEntry(int index, const QString& location, const QString& description, bool isDeprecated, const QString& deprecateHint);

    /**
     * \brief   Updates an entry by location.
     * \param   oldLocation     The old location of the entry.
     * \param   newLocation     The new location of the entry.
     * \param   description     The new description.
     * \param   isDeprecated    The new deprecated flag.
     * \param   deprecateHint   The new deprecation hint.
     * \return  True if the entry was updated, false otherwise.
     **/
    bool updateEntry(const QString & oldLocation, const QString& newLocation, const QString& description, bool isDeprecated, const QString& deprecateHint);

    /**
     * \brief   Removes an entry by location.
     * \param   location    The location of the entry to remove.
     * \return  True if the entry was removed, false otherwise.
     **/
    bool removeEntry(const QString& location);

    /**
     * \brief   Removes an entry by index.
     * \param   index   The index of the entry to remove.
     * \return  True if the entry was removed, false otherwise.
     **/
    bool removeEntry(int index);

    /**
     * \brief   Inserts a new entry at the specified location by shifting all other following items.
     * \param   index           The index of the entry to insert before.
     * \param   location        The new location.
     * \param   description     The new description.
     * \param   isDeprecated    The new deprecated flag.
     * \param   deprecateHint   The new deprecation hint.
     * \return  True if the entry was inserted, false otherwise.
     **/
    bool insertEntry(int index, const QString& location, const QString& description, bool isDeprecated, const QString& deprecateHint);

    /**
     * \brief   Inserts a new entry at the specified location by shifting all other following items.
     * \param   beforeLocation  The location to insert before.
     * \param   location        The new location.
     * \param   description     The new description.
     * \param   isDeprecated    The new deprecated flag.
     * \param   deprecateHint   The new deprecation hint.
     * \return  True if the entry was inserted, false otherwise.
     **/
    bool insertEntry(const QString& beforeLocation, const QString& location, const QString& description, bool isDeprecated, const QString& deprecateHint);

    /**
     * \brief   Sorts all entries.
     * \param   ascending   If true, sorts in ascending order, otherwise in descending order.
     **/
    void sortEntries(bool ascending);

    /**
     * \brief   Finds an entry by location.
     * \param   location    The location to search for.
     * \return  The index of the entry, or -1 if not found.
     **/
    int findEntry(const QString& location) const;
   
    /**
     * \brief   Returns the list of include entries.
     **/
    inline const QList<IncludeEntry> & entries(void) const;

//////////////////////////////////////////////////////////////////////////
// hidden members
//////////////////////////////////////////////////////////////////////////
private:
    SIIncludeData& mIncludeData;  //!< Reference to the SIIncludeData instance.
    QList<IncludeEntry> mEntries; //!< List of include entries.
};

//////////////////////////////////////////////////////////////////////////
// SIIncludeModel class inline functions
//////////////////////////////////////////////////////////////////////////

inline const QList<IncludeEntry> & SIIncludeModel::entries(void) const
{
    return mEntries;
}

#endif // LUSAN_MODEL_SI_SIINCLUDEMODEL_HPP
