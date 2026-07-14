#ifndef LUSAN_MODEL_SI_SICOMMAND_HPP
#define LUSAN_MODEL_SI_SICOMMAND_HPP
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
 *  \file        lusan/model/si/SICommand.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface command base — the shared DocCommand
 *               plus the SI document root, for commands that touch the whole document.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/model/common/DocCommand.hpp"

/************************************************************************
 * Dependencies
 ************************************************************************/
class ServiceInterfaceData;

/**
 * \class   SICommand
 * \brief   The Service-Interface-editor command base. It adds the concrete document root
 *          to the shared DocCommand so cross-section composite commands (for example
 *          deleting a request together with its response link) can reach it; the generic
 *          per-section add/remove/property/reorder commands use the shared templates
 *          directly and need only the notifier.
 **/
class SICommand : public DocCommand
{
//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
protected:
    SICommand(ServiceInterfaceData& data, DocModelNotifier& notifier, const QString& text, QUndoCommand* parent = nullptr);

//////////////////////////////////////////////////////////////////////////
// Attributes
//////////////////////////////////////////////////////////////////////////
protected:
    inline ServiceInterfaceData& data() const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    ServiceInterfaceData&   mData;
};

/**
 * \class   SICompositeCommand
 * \brief   An SI composite (one undo step of child commands) that also exposes the
 *          document root, so cross-section composites can build children that reach any
 *          section.
 **/
class SICompositeCommand : public SICommand
{
public:
    SICompositeCommand(ServiceInterfaceData& data, DocModelNotifier& notifier, const QString& text, QUndoCommand* parent = nullptr);
};

//////////////////////////////////////////////////////////////////////////
// SICommand inline methods
//////////////////////////////////////////////////////////////////////////

inline ServiceInterfaceData& SICommand::data() const
{
    return mData;
}

#endif  // LUSAN_MODEL_SI_SICOMMAND_HPP
