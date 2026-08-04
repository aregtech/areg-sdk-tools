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
#include "lusan/data/sm/SMMethodData.hpp"
#include "lusan/data/sm/SMOperation.hpp"
#include "lusan/data/sm/SMTransition.hpp"
#include "lusan/data/sm/StateMachineData.hpp"
#include "lusan/model/sm/SMDocumentIndex.hpp"

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

    // Only a trigger is a declared method, so only a trigger reads as a signature. Writing `evGo()`
    // for an event invented a method the model does not have.
    if (transition.getStimulusKind() != SMTransitionEntry::eStimulusKind::Trigger)
    {
        return name;
    }

    QStringList parts;
    const MethodBase* callee = SMDocumentIndex(data).method(name);
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
        return callSignature(call.getAction(), SMDocumentIndex(data).method(call.getAction()), call.getArguments());
    }

    case SMOperationBase::eOperation::EventSend:
    {
        // An event is signalled, not called: no parentheses and no payload here. The verb survives
        // because these surfaces carry no kind mark; the state box strips it and draws the bolt.
        return QStringLiteral("send ") + static_cast<const SMEventSend&>(op).getEvent();
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
