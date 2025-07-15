#ifndef LUSAN_MODEL_COMMON_TABLEMODELBASE_HPP
#define LUSAN_MODEL_COMMON_TABLEMODELBASE_HPP
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
 *  \file        lusan/model/common/TableModelBase.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Item model base class.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QAbstractTableModel>

/**
 * \brief   The base class for table models in the Lusan application.
 **/
class TableModelBase : public QAbstractTableModel
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructors / destructor
//////////////////////////////////////////////////////////////////////////
protected:
    TableModelBase(QObject* parent = nullptr);
    virtual ~TableModelBase(void) = default;

//////////////////////////////////////////////////////////////////////////
// Attributes
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Retrieves the 32-bit unique identifier for the model.
     **/
    inline uint32_t getModelId(void) const;

//////////////////////////////////////////////////////////////////////////
// Hidden members
//////////////////////////////////////////////////////////////////////////
private:
    const uint32_t mModelId; //!< The ID of the item model base.
};

//////////////////////////////////////////////////////////////////////////
// TableModelBase inline methods
//////////////////////////////////////////////////////////////////////////

inline uint32_t TableModelBase::getModelId(void) const
{
    return mModelId;
}

#endif  // LUSAN_MODEL_COMMON_TABLEMODELBASE_HPP
