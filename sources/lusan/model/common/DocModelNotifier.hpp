#ifndef LUSAN_MODEL_COMMON_DOCMODELNOTIFIER_HPP
#define LUSAN_MODEL_COMMON_DOCMODELNOTIFIER_HPP
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
 *  \file        lusan/model/common/DocModelNotifier.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, per-document change-notification hub shared by the
 *               Service Interface (.siml) and State Machine (.fsml) editors.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QObject>
#include <QList>
#include <QString>
#include <cstdint>

/**
 * \enum    eDocElementKind
 * \brief   The kind of a mutated element carried by every notifier signal, so a consumer
 *          can filter without dereferencing the model. The set is the union of the two
 *          editors' element kinds; a signal carries only an ID and this kind and consumers
 *          pull the current state from the model.
 **/
enum class eDocElementKind
{
      Overview      //!< The Overview section.
    , DataType      //!< A data type.
    , Attribute     //!< A machine/service attribute.
    , Event         //!< An event.
    , Timer         //!< A timer.
    , Method        //!< A method (SI request/response/broadcast, or FSM trigger/action/condition).
    , ResponseLink  //!< A service-interface request-to-response link.
    , Constant      //!< A constant.
    , Include       //!< An include.
    , Import        //!< A submachine import.
    , State         //!< A state.
    , Transition    //!< A transition.
    , Condition     //!< A condition row.
    , Operation     //!< An entry/exit/transition operation.
    , Note          //!< A layout note.
    , Layout        //!< A layout entry (view/node/edge).
};

/**
 * \class   DocModelNotifier
 * \brief   One QObject per open document that emits element-granular change signals.
 *          Commands are the sole emitters; every observer (page models, canvas, outline,
 *          properties, validation, dirty tracking) is a receiver. A single neutral
 *          emission point is required because undo replays mutations with no originating
 *          widget and several observers watch the same elements. Shared by both editors.
 **/
class DocModelNotifier : public QObject
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit DocModelNotifier(QObject* parent = nullptr);
    virtual ~DocModelNotifier() = default;

//////////////////////////////////////////////////////////////////////////
// Emit helpers (only commands call these)
//////////////////////////////////////////////////////////////////////////
public:
    void notifyElementAdded(uint32_t id, eDocElementKind kind);
    void notifyElementRemoved(uint32_t id, eDocElementKind kind);
    void notifyElementChanged(uint32_t id, eDocElementKind kind);
    void notifyListReordered(uint32_t ownerId, eDocElementKind kind);
    void notifyLayoutChanged(const QList<uint32_t>& ownerIds);
    void notifyNameChanged(uint32_t id, const QString& oldName, const QString& newName);
    void notifyDocumentReloaded();
    void notifyReadOnlyChanged(bool readOnly);

//////////////////////////////////////////////////////////////////////////
// Signals
//////////////////////////////////////////////////////////////////////////
signals:
    void elementAdded(uint32_t id, eDocElementKind kind);
    void elementRemoved(uint32_t id, eDocElementKind kind);
    void elementChanged(uint32_t id, eDocElementKind kind);
    void listReordered(uint32_t ownerId, eDocElementKind kind);
    void layoutChanged(const QList<uint32_t>& ownerIds);
    void nameChanged(uint32_t id, const QString& oldName, const QString& newName);
    void documentReloaded();
    void readOnlyChanged(bool readOnly);
};

#endif  // LUSAN_MODEL_COMMON_DOCMODELNOTIFIER_HPP
