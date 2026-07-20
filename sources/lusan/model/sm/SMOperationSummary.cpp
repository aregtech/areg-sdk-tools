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
 *  \file        lusan/model/sm/SMOperationSummary.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM operation one-line signature renderer (headless).
 *
 ************************************************************************/

#include "lusan/model/sm/SMOperationSummary.hpp"

#include "lusan/data/common/MethodBase.hpp"
#include "lusan/data/sm/SMEventData.hpp"
#include "lusan/data/sm/SMMethodData.hpp"
#include "lusan/data/sm/SMOperation.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"

#include <QStringList>

namespace
{
    //!< The displayed text of one mapped argument (referenced name, literal, or code marker).
    QString argumentText(const SMArgumentEntry& arg)
    {
        switch (arg.getSource())
        {
        case SMArgumentEntry::eValueSource::Value:
            return arg.getValue().isEmpty() ? QStringLiteral("?") : arg.getValue();
        case SMArgumentEntry::eValueSource::Expression:
        case SMArgumentEntry::eValueSource::Lambda:
            return QStringLiteral("{..}");
        default:
            return arg.getValue().isEmpty() ? QStringLiteral("?") : arg.getValue();
        }
    }

    //!< The mapped argument for a parameter name, or nullptr when the parameter is unmapped.
    const SMArgumentEntry* argFor(const QList<SMArgumentEntry>& args, const QString& name)
    {
        for (const SMArgumentEntry& arg : args)
        {
            if (arg.getName() == name)
            {
                return &arg;
            }
        }

        return nullptr;
    }

    //!< Renders `name(a, b, c)` from a callee signature and the operation's argument mappings.
    QString callSignature(const QString& name, const MethodBase* callee, const QList<SMArgumentEntry>& args)
    {
        if (callee == nullptr)
        {
            return name + QStringLiteral("(...)");
        }

        QStringList parts;
        for (const MethodParameter& param : callee->getElements())
        {
            const SMArgumentEntry* mapped = argFor(args, param.getName());
            if (mapped != nullptr)
            {
                parts.append(argumentText(*mapped));
            }
            else if (param.hasDefault())
            {
                parts.append(param.getValue());
            }
            else
            {
                parts.append(param.getName());
            }
        }

        return name + QLatin1Char('(') + parts.join(QStringLiteral(", ")) + QLatin1Char(')');
    }

    //!< The Action-typed method with the given name, or nullptr.
    const MethodBase* findAction(const StateMachineData& data, const QString& name)
    {
        for (const SMMethodEntry* method : data.getMethods().getElements())
        {
            if ((method != nullptr) && (method->getName() == name))
            {
                return method;
            }
        }

        return nullptr;
    }

    //!< The event with the given name, or nullptr.
    const MethodBase* findEvent(const StateMachineData& data, const QString& name)
    {
        for (const SMEventEntry* event : data.getEvents().getElements())
        {
            if ((event != nullptr) && (event->getName() == name))
            {
                return event;
            }
        }

        return nullptr;
    }

    //!< The first non-empty, trimmed line of an inline body, elided for the row.
    QString firstLine(const QString& body)
    {
        const QStringList lines = body.split(QLatin1Char('\n'));
        for (const QString& line : lines)
        {
            const QString trimmed = line.trimmed();
            if (trimmed.isEmpty() == false)
            {
                return (trimmed.length() > 40) ? (trimmed.left(37) + QStringLiteral("...")) : trimmed;
            }
        }

        return QString();
    }
}

QString SMOperationSummary::stimulusSignature(const StateMachineData& data, const SMTransitionEntry& transition)
{
    const QString name = transition.getStimulus();
    if (name.isEmpty())
    {
        return name;
    }

    // A timer expiry carries no parameters; a trigger or event reads as a method signature.
    if (transition.getStimulusKind() == SMTransitionEntry::eStimulusKind::Timer)
    {
        return name;
    }

    const MethodBase* callee = (transition.getStimulusKind() == SMTransitionEntry::eStimulusKind::Event)
        ? findEvent(data, name)
        : findAction(data, name);
    if (callee == nullptr)
    {
        // Trigger methods live in the same registry; try there before giving up.
        callee = findAction(data, name);
    }

    QStringList parts;
    if (callee != nullptr)
    {
        for (const MethodParameter& param : callee->getElements())
        {
            parts.append(param.getName());
        }
    }

    return name + QLatin1Char('(') + parts.join(QStringLiteral(", ")) + QLatin1Char(')');
}

QString SMOperationSummary::text(const StateMachineData& data, const SMOperationBase& op)
{
    switch (op.getOperationType())
    {
    case SMOperationBase::eOperation::ActionCall:
    {
        const SMActionCall& call = static_cast<const SMActionCall&>(op);
        return callSignature(call.getAction(), findAction(data, call.getAction()), call.getArguments());
    }

    case SMOperationBase::eOperation::EventSend:
    {
        const SMEventSend& send = static_cast<const SMEventSend&>(op);
        return QStringLiteral("send ") + callSignature(send.getEvent(), findEvent(data, send.getEvent()), send.getArguments());
    }

    case SMOperationBase::eOperation::AttributeSet:
    {
        const SMAttributeSet& set = static_cast<const SMAttributeSet&>(op);
        QString value = (set.getSource() == SMArgumentEntry::eValueSource::Expression) ? QStringLiteral("{..}") : set.getValue();
        if (value.isEmpty())
        {
            value = QStringLiteral("?");
        }

        return set.getAttribute() + QStringLiteral(" = ") + value;
    }

    case SMOperationBase::eOperation::TimerStart:
    {
        const SMTimerStart& timer = static_cast<const SMTimerStart&>(op);
        QStringList overrides;
        if (timer.hasTimeoutOverride())
        {
            overrides.append(QStringLiteral("%1 ms").arg(timer.getTimeout()));
        }
        if (timer.hasRepeatOverride())
        {
            overrides.append((timer.getRepeat() == 0) ? QStringLiteral("repeat") : QStringLiteral("x%1").arg(timer.getRepeat()));
        }

        QString result = QStringLiteral("start ") + timer.getTimer();
        if (overrides.isEmpty() == false)
        {
            result += QStringLiteral(" (") + overrides.join(QStringLiteral(", ")) + QLatin1Char(')');
        }

        return result;
    }

    case SMOperationBase::eOperation::TimerStop:
        return QStringLiteral("stop ") + static_cast<const SMTimerStop&>(op).getTimer();

    case SMOperationBase::eOperation::InlineCode:
    default:
    {
        const QString line = firstLine(static_cast<const SMInlineCode&>(op).getBody());
        return line.isEmpty() ? QStringLiteral("{ inline code }") : (QStringLiteral("{ ") + line + QStringLiteral(" }"));
    }
    }
}
