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
 *  \copyright   (c) 2023-2026 Aregtech (Artak Avetyan).
 *  \file        lusan/data/sm/SMClipboard.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM copy/paste clipboard payload.
 *
 ************************************************************************/

#include "lusan/data/sm/SMClipboard.hpp"

#include "lusan/common/XmlSM.hpp"
#include "lusan/data/sm/StateMachineData.hpp"

#include <QRegularExpression>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <functional>

namespace
{
    // Clipboard-envelope element names; everything inside reuses the .fsml vocabulary.
    constexpr const char* const _elemClipboard  { "FSMClipboard" };
    constexpr const char* const _elemElements   { "Elements" };
    constexpr const char* const _elemReferences { "References" };
    constexpr const char* const _formatVersion  { "1.0.0" };

    //!< Writes one wrapped registry section from individually selected entries.
    template<typename Entry>
    void _writeSection(QXmlStreamWriter& xml, QLatin1StringView sectionName, const QList<const Entry*>& entries)
    {
        if (entries.isEmpty())
        {
            return;
        }

        xml.writeStartElement(sectionName);
        for (const Entry* entry : entries)
        {
            entry->writeToXml(xml);
        }

        xml.writeEndElement();
    }

    //!< Resolves the referenced names of one registry into entry pointers, skipping
    //!< the names already carried as explicitly copied entries.
    template<typename Entry, typename Lookup>
    QList<const Entry*> _resolveReferences(const QSet<QString>& names, const QSet<QString>& explicitNames, Lookup lookup)
    {
        QList<const Entry*> result;
        for (const QString& name : names)
        {
            if (explicitNames.contains(name))
            {
                continue;
            }

            const Entry* entry = lookup(name);
            if (entry != nullptr)
            {
                result.append(entry);
            }
        }

        return result;
    }

    void _collectOperationRefs(const SMOperationList& operations, SMClipboard::ReferenceNames& out);
    void _collectArgumentRefs(const QList<SMArgumentEntry>& arguments, SMClipboard::ReferenceNames& out);

    void _collectArgumentRefs(const QList<SMArgumentEntry>& arguments, SMClipboard::ReferenceNames& out)
    {
        for (const SMArgumentEntry& arg : arguments)
        {
            switch (arg.getSource())
            {
            case SMArgumentEntry::eValueSource::Attribute:  out.attributes.insert(arg.getValue()); break;
            case SMArgumentEntry::eValueSource::Constant:   out.constants.insert(arg.getValue());  break;
            case SMArgumentEntry::eValueSource::Condition:  out.methods.insert(arg.getValue());    break;
            default: break;
            }
        }
    }

    void _collectOperationRefs(const SMOperationList& operations, SMClipboard::ReferenceNames& out)
    {
        for (const SMOperationBase* op : operations.getOperations())
        {
            switch (op->getOperationType())
            {
            case SMOperationBase::eOperation::ActionCall:
            {
                const SMActionCall* call = static_cast<const SMActionCall*>(op);
                out.methods.insert(call->getAction());
                _collectArgumentRefs(call->getArguments(), out);
                break;
            }
            case SMOperationBase::eOperation::AttributeSet:
            {
                const SMAttributeSet* set = static_cast<const SMAttributeSet*>(op);
                out.attributes.insert(set->getAttribute());
                switch (set->getSource())
                {
                case SMArgumentEntry::eValueSource::Attribute:  out.attributes.insert(set->getValue()); break;
                case SMArgumentEntry::eValueSource::Constant:   out.constants.insert(set->getValue());  break;
                case SMArgumentEntry::eValueSource::Condition:  out.methods.insert(set->getValue());    break;
                default: break;
                }
                break;
            }
            case SMOperationBase::eOperation::TimerStart:
                out.timers.insert(static_cast<const SMTimerStart*>(op)->getTimer());
                break;
            case SMOperationBase::eOperation::TimerStop:
                out.timers.insert(static_cast<const SMTimerStop*>(op)->getTimer());
                break;
            case SMOperationBase::eOperation::EventSend:
            {
                const SMEventSend* send = static_cast<const SMEventSend*>(op);
                out.events.insert(send->getEvent());
                _collectArgumentRefs(send->getArguments(), out);
                break;
            }
            case SMOperationBase::eOperation::InlineCode:
            default:
                break;
            }
        }
    }

    //!< Remaps one referenced name in place; no-op when the map has no entry.
    void _remapName(const QHash<QString, QString>& renames, const QString& name, const std::function<void(const QString&)>& setter)
    {
        const auto found = renames.constFind(name);
        if (found != renames.constEnd())
        {
            setter(found.value());
        }
    }

    void _remapArguments(QList<SMArgumentEntry>& arguments, const SMClipboard::RenameMaps& renames)
    {
        for (SMArgumentEntry& arg : arguments)
        {
            const QHash<QString, QString>* map = nullptr;
            switch (arg.getSource())
            {
            case SMArgumentEntry::eValueSource::Attribute:  map = &renames.attributes; break;
            case SMArgumentEntry::eValueSource::Constant:   map = &renames.constants;  break;
            case SMArgumentEntry::eValueSource::Condition:  map = &renames.methods;    break;
            default: break;
            }

            if (map != nullptr)
            {
                _remapName(*map, arg.getValue(), [&arg](const QString& name) { arg.setValue(name); });
            }
        }
    }

    void _remapOperations(SMOperationList& operations, const SMClipboard::RenameMaps& renames)
    {
        for (SMOperationBase* op : operations.getOperations())
        {
            switch (op->getOperationType())
            {
            case SMOperationBase::eOperation::ActionCall:
            {
                SMActionCall* call = static_cast<SMActionCall*>(op);
                _remapName(renames.methods, call->getAction(), [call](const QString& name) { call->setAction(name); });
                _remapArguments(call->getArguments(), renames);
                break;
            }
            case SMOperationBase::eOperation::AttributeSet:
            {
                SMAttributeSet* set = static_cast<SMAttributeSet*>(op);
                _remapName(renames.attributes, set->getAttribute(), [set](const QString& name) { set->setAttribute(name); });
                const QHash<QString, QString>* map = nullptr;
                switch (set->getSource())
                {
                case SMArgumentEntry::eValueSource::Attribute:  map = &renames.attributes; break;
                case SMArgumentEntry::eValueSource::Constant:   map = &renames.constants;  break;
                case SMArgumentEntry::eValueSource::Condition:  map = &renames.methods;    break;
                default: break;
                }
                if (map != nullptr)
                {
                    _remapName(*map, set->getValue(), [set](const QString& name) { set->setValue(name); });
                }
                break;
            }
            case SMOperationBase::eOperation::TimerStart:
            {
                SMTimerStart* start = static_cast<SMTimerStart*>(op);
                _remapName(renames.timers, start->getTimer(), [start](const QString& name) { start->setTimer(name); });
                break;
            }
            case SMOperationBase::eOperation::TimerStop:
            {
                SMTimerStop* stop = static_cast<SMTimerStop*>(op);
                _remapName(renames.timers, stop->getTimer(), [stop](const QString& name) { stop->setTimer(name); });
                break;
            }
            case SMOperationBase::eOperation::EventSend:
            {
                SMEventSend* send = static_cast<SMEventSend*>(op);
                _remapName(renames.events, send->getEvent(), [send](const QString& name) { send->setEvent(name); });
                _remapArguments(send->getArguments(), renames);
                break;
            }
            case SMOperationBase::eOperation::InlineCode:
            default:
                break;
            }
        }
    }

}

//////////////////////////////////////////////////////////////////////////
// SMClipboardContent implementation
//////////////////////////////////////////////////////////////////////////

SMClipboardContent::ClipRoot::ClipRoot()
    : ElementBase(static_cast<ElementBase*>(nullptr))
{
}

SMClipboardContent::SMClipboardContent()
    : mRoot         ( )
    , mStates       (&mRoot)
    , mLayout       (&mRoot)
    , mDataTypes    (&mRoot)
    , mAttributes   (&mRoot)
    , mEvents       (&mRoot)
    , mTimers       (&mRoot)
    , mMethods      (&mRoot)
    , mConstants    (&mRoot)
    , mIncludes     (&mRoot)
    , mRefAttributes(&mRoot)
    , mRefEvents    (&mRoot)
    , mRefTimers    (&mRoot)
    , mRefMethods   (&mRoot)
    , mRefConstants (&mRoot)
{
}

bool SMClipboardContent::isEmpty() const
{
    return  mStates.isEmpty()
        &&  mLayout.getNotes().isEmpty()
        &&  mDataTypes.isEmpty()
        &&  mAttributes.isEmpty()
        &&  mEvents.isEmpty()
        &&  mTimers.isEmpty()
        &&  mMethods.isEmpty()
        &&  mConstants.isEmpty()
        &&  mIncludes.isEmpty();
}

//////////////////////////////////////////////////////////////////////////
// SMClipboard implementation
//////////////////////////////////////////////////////////////////////////

QString SMClipboard::serialize(const StateMachineData& doc, const QList<uint32_t>& elementIds)
{
    // Classify the selection. Start states are skipped: a pasted level may never
    // receive a second Start. States nested inside another selected state are covered
    // by their ancestor's subtree and are skipped as well.
    QList<const SMStateEntry*>      states;
    QList<uint32_t>                 noteIds;
    QList<const DataTypeCustom*>    dataTypes;
    QList<const SMAttributeEntry*>  attributes;
    QList<const SMEventEntry*>      events;
    QList<const SMTimerEntry*>      timers;
    QList<const SMMethodEntry*>     methods;
    QList<const ConstantEntry*>     constants;
    QList<const IncludeEntry*>      includes;

    for (uint32_t id : elementIds)
    {
        const SMStateEntry* state = doc.findStateById(id);
        if (state != nullptr)
        {
            if (state->getKind() != SMStateEntry::eStateKind::Start)
            {
                states.append(state);
            }

            continue;
        }

        if (doc.getLayout().findNote(id) != nullptr)
        {
            noteIds.append(id);
            continue;
        }

        if (DataTypeCustom** dataType = doc.getDataTypes().findElement(id); dataType != nullptr)
        {
            dataTypes.append(*dataType);
        }
        else if (const SMAttributeEntry* attribute = doc.getAttributes().findElement(id); attribute != nullptr)
        {
            attributes.append(attribute);
        }
        else if (const SMEventEntry* event = doc.getEvents().findEvent(id); event != nullptr)
        {
            events.append(event);
        }
        else if (const SMTimerEntry* timer = doc.getTimers().findElement(id); timer != nullptr)
        {
            timers.append(timer);
        }
        else if (const SMMethodEntry* method = doc.getMethods().findMethod(id); method != nullptr)
        {
            methods.append(method);
        }
        else if (const ConstantEntry* constant = doc.getConstants().findElement(id); constant != nullptr)
        {
            constants.append(constant);
        }
        else if (const IncludeEntry* include = doc.getIncludes().findElement(id); include != nullptr)
        {
            includes.append(include);
        }
    }

    // Drop states whose ancestor is also selected: the ancestor's subtree carries them.
    for (int i = states.size() - 1; i >= 0; --i)
    {
        const uint32_t id = states.at(i)->getId();
        for (const SMStateEntry* other : states)
        {
            if ((other != states.at(i)) && other->hasNestedStates()
                && (other->getNestedStates()->findStateByIdRecursive(id) != nullptr))
            {
                states.removeAt(i);
                break;
            }
        }
    }

    if (states.isEmpty() && noteIds.isEmpty() && dataTypes.isEmpty() && attributes.isEmpty()
        && events.isEmpty() && timers.isEmpty() && methods.isEmpty() && constants.isEmpty()
        && includes.isEmpty())
    {
        return QString();
    }

    // The layout of the copied set: Node/Edge entries of every state and transition in
    // the subtrees, the notes bound to any of them, and the explicitly selected notes.
    QList<uint32_t> ownedIds;
    ReferenceNames references;
    for (const SMStateEntry* state : states)
    {
        collectOwnedIds(*state, ownedIds);
        collectReferences(*state, references);
    }

    SMLayoutData layoutOut(nullptr);
    const SMLayoutData& layout = doc.getLayout();
    for (uint32_t owner : ownedIds)
    {
        if (const SMLayoutNode* node = layout.findNode(owner); node != nullptr)
        {
            layoutOut.addNode(owner) = *node;
        }

        if (const SMLayoutEdge* edge = layout.findEdge(owner); edge != nullptr)
        {
            layoutOut.addEdge(owner) = *edge;
        }
    }

    for (const SMLayoutNote& note : layout.getNotes())
    {
        if (noteIds.contains(note.id) || ((note.owner != 0u) && ownedIds.contains(note.owner)))
        {
            layoutOut.restoreNote(note);
        }
    }

    // Referenced registry entries, minus those already explicitly copied.
    QSet<QString> explicitNames;
    for (const SMEventEntry* entry : events)
    {
        explicitNames.insert(entry->getName());
    }
    const QList<const SMEventEntry*> refEvents = _resolveReferences<SMEventEntry>(references.events, explicitNames
            , [&doc](const QString& name) { return doc.getEvents().findEvent(name); });

    explicitNames.clear();
    for (const SMTimerEntry* entry : timers)
    {
        explicitNames.insert(entry->getName());
    }
    const QList<const SMTimerEntry*> refTimers = _resolveReferences<SMTimerEntry>(references.timers, explicitNames
            , [&doc](const QString& name) { return doc.getTimers().findElement(name); });

    explicitNames.clear();
    for (const SMMethodEntry* entry : methods)
    {
        explicitNames.insert(entry->getName());
    }
    const QList<const SMMethodEntry*> refMethods = _resolveReferences<SMMethodEntry>(references.methods, explicitNames
            , [&doc](const QString& name) { return doc.getMethods().findMethod(name); });

    explicitNames.clear();
    for (const SMAttributeEntry* entry : attributes)
    {
        explicitNames.insert(entry->getName());
    }
    const QList<const SMAttributeEntry*> refAttributes = _resolveReferences<SMAttributeEntry>(references.attributes, explicitNames
            , [&doc](const QString& name) { return doc.getAttributes().findElement(name); });

    explicitNames.clear();
    for (const ConstantEntry* entry : constants)
    {
        explicitNames.insert(entry->getName());
    }
    const QList<const ConstantEntry*> refConstants = _resolveReferences<ConstantEntry>(references.constants, explicitNames
            , [&doc](const QString& name) { return doc.getConstants().findElement(name); });

    QString out;
    QXmlStreamWriter xml(&out);
    xml.setAutoFormatting(true);
    xml.setAutoFormattingIndent(4);
    xml.writeStartDocument();
    xml.writeStartElement(_elemClipboard);
    xml.writeAttribute(XmlSM::xmlSMAttributeFormatVersion, _formatVersion);

    xml.writeStartElement(_elemElements);
    if (states.isEmpty() == false)
    {
        xml.writeStartElement(XmlSM::xmlSMElementStateList);
        for (const SMStateEntry* state : states)
        {
            state->writeToXml(xml);
        }

        xml.writeEndElement();
    }

    _writeSection(xml, XmlSM::xmlSMElementDataTypeList, dataTypes);
    _writeSection(xml, XmlSM::xmlSMElementAttributeList, attributes);
    _writeSection(xml, XmlSM::xmlSMElementEventList, events);
    _writeSection(xml, XmlSM::xmlSMElementTimerList, timers);
    _writeSection(xml, XmlSM::xmlSMElementMethodList, methods);
    _writeSection(xml, XmlSM::xmlSMElementConstantList, constants);
    _writeSection(xml, XmlSM::xmlSMElementIncludeList, includes);
    xml.writeEndElement();  // Elements

    if ((refEvents.isEmpty() && refTimers.isEmpty() && refMethods.isEmpty()
         && refAttributes.isEmpty() && refConstants.isEmpty()) == false)
    {
        xml.writeStartElement(_elemReferences);
        _writeSection(xml, XmlSM::xmlSMElementAttributeList, refAttributes);
        _writeSection(xml, XmlSM::xmlSMElementEventList, refEvents);
        _writeSection(xml, XmlSM::xmlSMElementTimerList, refTimers);
        _writeSection(xml, XmlSM::xmlSMElementMethodList, refMethods);
        _writeSection(xml, XmlSM::xmlSMElementConstantList, refConstants);
        xml.writeEndElement();  // References
    }

    layoutOut.writeToXml(xml);
    xml.writeEndElement();  // FSMClipboard
    xml.writeEndDocument();
    return out;
}

std::unique_ptr<SMClipboardContent> SMClipboard::parse(const QString& xml)
{
    std::unique_ptr<SMClipboardContent> content = std::make_unique<SMClipboardContent>();
    QXmlStreamReader reader(xml);

    bool foundRoot { false };
    bool inReferences { false };
    while (reader.atEnd() == false)
    {
        reader.readNext();
        if (reader.tokenType() == QXmlStreamReader::StartElement)
        {
            const auto name = reader.name();
            if (foundRoot == false)
            {
                if (name != QLatin1String(_elemClipboard))
                {
                    return nullptr;
                }

                foundRoot = true;
            }
            else if (name == QLatin1String(_elemElements))
            {
                inReferences = false;
            }
            else if (name == QLatin1String(_elemReferences))
            {
                inReferences = true;
            }
            else if (name == QLatin1String(XmlSM::xmlSMElementLayout))
            {
                content->mLayout.readFromXml(reader);
            }
            else if (name == QLatin1String(XmlSM::xmlSMElementStateList))
            {
                content->mStates.readFromXml(reader);
            }
            else if (name == QLatin1String(XmlSM::xmlSMElementDataTypeList))
            {
                content->mDataTypes.readFromXml(reader);
            }
            else if (name == QLatin1String(XmlSM::xmlSMElementAttributeList))
            {
                (inReferences ? content->mRefAttributes : content->mAttributes).readFromXml(reader);
            }
            else if (name == QLatin1String(XmlSM::xmlSMElementEventList))
            {
                (inReferences ? content->mRefEvents : content->mEvents).readFromXml(reader);
            }
            else if (name == QLatin1String(XmlSM::xmlSMElementTimerList))
            {
                (inReferences ? content->mRefTimers : content->mTimers).readFromXml(reader);
            }
            else if (name == QLatin1String(XmlSM::xmlSMElementMethodList))
            {
                (inReferences ? content->mRefMethods : content->mMethods).readFromXml(reader);
            }
            else if (name == QLatin1String(XmlSM::xmlSMElementConstantList))
            {
                (inReferences ? content->mRefConstants : content->mConstants).readFromXml(reader);
            }
            else if (name == QLatin1String(XmlSM::xmlSMElementIncludeList))
            {
                content->mIncludes.readFromXml(reader);
            }
        }
        else if (reader.tokenType() == QXmlStreamReader::EndElement)
        {
            if (reader.name() == QLatin1String(_elemReferences))
            {
                inReferences = false;
            }
        }
    }

    if ((foundRoot == false) || reader.hasError())
    {
        return nullptr;
    }

    return content;
}

void SMClipboard::reallocateIds(SMStateEntry& state, const ElementBase& counter, QHash<uint32_t, uint32_t>& oldToNew)
{
    const auto realloc = [&counter, &oldToNew](const ElementBase& elem)
    {
        const uint32_t oldId = elem.getId();
        elem.setId(counter.getNextId());
        oldToNew.insert(oldId, elem.getId());
    };

    const auto reallocArgs = [&realloc](QList<SMArgumentEntry>& args)
    {
        for (SMArgumentEntry& arg : args)
        {
            realloc(arg);
        }
    };

    const auto reallocOps = [&realloc, &reallocArgs](SMOperationList& ops)
    {
        for (SMOperationBase* op : ops.getOperations())
        {
            realloc(*op);
            switch (op->getOperationType())
            {
            case SMOperationBase::eOperation::ActionCall:
                reallocArgs(static_cast<SMActionCall*>(op)->getArguments());
                break;
            case SMOperationBase::eOperation::EventSend:
                reallocArgs(static_cast<SMEventSend*>(op)->getArguments());
                break;
            default:
                break;
            }
        }
    };

    realloc(state);
    reallocOps(state.getEntryList());
    reallocOps(state.getExitList());
    for (SMTransitionEntry* transition : state.getTransitions().getElements())
    {
        realloc(*transition);
        for (SMConditionEntry* condition : transition->getConditions().collectLeaves())
        {
            realloc(*condition);
            reallocArgs(condition->getArguments());
        }
        for (SMConditionGroup* group : transition->getConditions().collectGroups())
        {
            realloc(*group);
        }

        reallocOps(transition->getOperations());
    }

    if (state.hasNestedStates())
    {
        for (SMStateEntry* nested : state.getNestedStates()->getElements())
        {
            reallocateIds(*nested, counter, oldToNew);
        }
    }
}

void SMClipboard::collectStates(SMStateEntry& state, QList<SMStateEntry*>& out)
{
    out.append(&state);
    if (state.hasNestedStates())
    {
        for (SMStateEntry* nested : state.getNestedStates()->getElements())
        {
            collectStates(*nested, out);
        }
    }
}

void SMClipboard::collectOwnedIds(const SMStateEntry& state, QList<uint32_t>& out)
{
    out.append(state.getId());
    for (const SMTransitionEntry* transition : state.getTransitions().getElements())
    {
        out.append(transition->getId());
    }

    if (state.hasNestedStates())
    {
        for (const SMStateEntry* nested : state.getNestedStates()->getElements())
        {
            collectOwnedIds(*nested, out);
        }
    }
}

void SMClipboard::collectReferences(const SMStateEntry& state, ReferenceNames& out)
{
    if (state.getOnFinal().isEmpty() == false)
    {
        out.events.insert(state.getOnFinal());
    }

    _collectOperationRefs(state.getEntryList(), out);
    _collectOperationRefs(state.getExitList(), out);
    for (const SMTransitionEntry* transition : state.getTransitions().getElements())
    {
        if (transition->getStimulus().isEmpty() == false)
        {
            switch (transition->getStimulusKind())
            {
            case SMTransitionEntry::eStimulusKind::Trigger: out.methods.insert(transition->getStimulus()); break;
            case SMTransitionEntry::eStimulusKind::Event:   out.events.insert(transition->getStimulus());  break;
            case SMTransitionEntry::eStimulusKind::Timer:   out.timers.insert(transition->getStimulus());  break;
            default: break;
            }
        }

        for (const SMConditionEntry* condition : transition->getConditions().collectLeaves())
        {
            const auto collectOperand = [&out](SMConditionEntry::eOperandKind kind, const QString& name)
            {
                if (name.isEmpty())
                {
                    return;
                }

                switch (kind)
                {
                case SMArgumentEntry::eValueSource::Attribute:  out.attributes.insert(name); break;
                case SMArgumentEntry::eValueSource::Constant:   out.constants.insert(name);  break;
                case SMArgumentEntry::eValueSource::Condition:  out.methods.insert(name);    break;
                default: break;
                }
            };

            collectOperand(condition->getLhsKind(), condition->getLhs());
            collectOperand(condition->getRhsKind(), condition->getRhs());
            _collectArgumentRefs(condition->getArguments(), out);
        }

        _collectOperationRefs(transition->getOperations(), out);
    }

    if (state.hasNestedStates())
    {
        for (const SMStateEntry* nested : state.getNestedStates()->getElements())
        {
            collectReferences(*nested, out);
        }
    }
}

void SMClipboard::remapReferences(SMStateEntry& state, const RenameMaps& renames)
{
    _remapName(renames.events, state.getOnFinal(), [&state](const QString& name) { state.setOnFinal(name); });
    _remapOperations(state.getEntryList(), renames);
    _remapOperations(state.getExitList(), renames);
    for (SMTransitionEntry* transition : state.getTransitions().getElements())
    {
        const QHash<QString, QString>* stimulusMap = nullptr;
        switch (transition->getStimulusKind())
        {
        case SMTransitionEntry::eStimulusKind::Trigger: stimulusMap = &renames.methods; break;
        case SMTransitionEntry::eStimulusKind::Event:   stimulusMap = &renames.events;  break;
        case SMTransitionEntry::eStimulusKind::Timer:   stimulusMap = &renames.timers;  break;
        default: break;
        }

        if (stimulusMap != nullptr)
        {
            _remapName(*stimulusMap, transition->getStimulus(), [transition](const QString& name) { transition->setStimulus(name); });
        }

        if (transition->isExternal())
        {
            _remapName(renames.states, transition->getTo(), [transition](const QString& name) { transition->setTo(name); });
        }

        for (SMConditionEntry* condition : transition->getConditions().collectLeaves())
        {
            const auto remapOperand = [&renames](SMConditionEntry::eOperandKind kind, const QString& name, const std::function<void(const QString&)>& setter)
            {
                switch (kind)
                {
                case SMArgumentEntry::eValueSource::Attribute:  _remapName(renames.attributes, name, setter); break;
                case SMArgumentEntry::eValueSource::Constant:   _remapName(renames.constants, name, setter);  break;
                case SMArgumentEntry::eValueSource::Condition:  _remapName(renames.methods, name, setter);    break;
                default: break;
                }
            };

            remapOperand(condition->getLhsKind(), condition->getLhs(), [condition](const QString& name) { condition->setLhs(name); });
            remapOperand(condition->getRhsKind(), condition->getRhs(), [condition](const QString& name) { condition->setRhs(name); });
            _remapArguments(condition->getArguments(), renames);
        }

        _remapOperations(transition->getOperations(), renames);
    }

    if (state.hasNestedStates())
    {
        for (SMStateEntry* nested : state.getNestedStates()->getElements())
        {
            remapReferences(*nested, renames);
        }
    }
}

bool SMClipboard::structurallyEqual(const DocumentElem& lhs, const DocumentElem& rhs)
{
    const auto dump = [](const DocumentElem& elem) -> QString
    {
        QString out;
        QXmlStreamWriter writer(&out);
        elem.writeToXml(writer);

        // Element IDs differ between documents by design; compare everything else.
        static const QRegularExpression idAttribute(QStringLiteral(" ID=\"\\d+\""));
        return out.remove(idAttribute);
    };

    return dump(lhs) == dump(rhs);
}
