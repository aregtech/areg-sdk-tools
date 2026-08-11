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
 *  \file        lusan/common/DocSchemaReader.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, reads a document schema into the elements it declares.
 *
 ************************************************************************/

#include "lusan/common/DocSchemaReader.hpp"

#include <QCoreApplication>
#include <QFile>
#include <QHash>
#include <QXmlStreamReader>

#include <algorithm>

namespace
{
    //!< A group referencing a group referencing a group is already further than these schemas
    //!< go; the bound is here so a schema that refers to itself cannot spin.
    constexpr int MAX_GROUP_DEPTH { 8 };

    //!< The scope an element declaration was found in. The name it carries decides who the
    //!< declared element's parent turns out to be.
    enum class eScopeKind
    {
          Document      //!< Outside any type: the declaration is the document element.
        , Element       //!< Inside an element with a type written out in place.
        , Type          //!< Inside a named type: the parents are whoever uses that type.
        , Group         //!< Inside a named group: the parents are whoever refers to the group.
    };

    struct Scope
    {
        eScopeKind  kind { eScopeKind::Document };
        QString     name;
    };

    //!< Scopes are compared and looked up as one string, so the three kinds cannot collide.
    QString scopeKey(const Scope& scope)
    {
        switch (scope.kind)
        {
        case eScopeKind::Element:
            return QStringLiteral("e:") + scope.name;

        case eScopeKind::Type:
            return QStringLiteral("t:") + scope.name;

        case eScopeKind::Group:
            return QStringLiteral("g:") + scope.name;

        default:
            return QString();
        }
    }

    //!< Everything one pass over the schema collects, before any of it is resolved.
    struct Collected
    {
        QHash<QString, QStringList> children;       //!< Scope key -> the elements declared in it.
        QHash<QString, QStringList> groupRefs;      //!< Scope key -> the groups it pulls in.
        QHash<QString, QStringList> scopesOfType;   //!< Type name -> the elements written with it.
        QString                     root;           //!< The document element.
    };

    //!< True for a type written by the schema language itself, which declares no elements.
    bool isBuiltInType(QStringView type)
    {
        return type.contains(QLatin1Char(':'));
    }

    void appendUnique(QStringList& list, const QString& value)
    {
        if (list.contains(value) == false)
        {
            list.append(value);
        }
    }

    void collect(QXmlStreamReader& xml, Collected& out)
    {
        QList<Scope> stack;

        while (xml.atEnd() == false)
        {
            const QXmlStreamReader::TokenType token = xml.readNext();
            if (token == QXmlStreamReader::EndElement)
            {
                if (stack.isEmpty() == false)
                {
                    stack.removeLast();
                }
                continue;
            }

            if (token != QXmlStreamReader::StartElement)
            {
                continue;
            }

            // Anything the schema language declares that is not one of the four below leaves the
            // scope where it was: a sequence, a choice or an annotation holds declarations but
            // does not own them.
            Scope scope = stack.isEmpty() ? Scope() : stack.constLast();
            const QStringView tag = xml.name();
            const QString name    = xml.attributes().value(QLatin1StringView("name")).toString();

            if (tag == QLatin1StringView("element"))
            {
                if (name.isEmpty() == false)
                {
                    if (scope.kind == eScopeKind::Document)
                    {
                        out.root = name;
                    }
                    else
                    {
                        appendUnique(out.children[scopeKey(scope)], name);
                    }

                    const QStringView type = xml.attributes().value(QLatin1StringView("type"));
                    if ((type.isEmpty() == false) && (isBuiltInType(type) == false))
                    {
                        appendUnique(out.scopesOfType[type.toString()], name);
                    }

                    scope.kind = eScopeKind::Element;
                    scope.name = name;
                }
            }
            else if (tag == QLatin1StringView("complexType"))
            {
                // A type written out inside an element belongs to that element; a named one is
                // shared, so who uses it decides.
                if (name.isEmpty() == false)
                {
                    scope.kind = eScopeKind::Type;
                    scope.name = name;
                }
            }
            else if (tag == QLatin1StringView("group"))
            {
                const QStringView ref = xml.attributes().value(QLatin1StringView("ref"));
                if (name.isEmpty() == false)
                {
                    scope.kind = eScopeKind::Group;
                    scope.name = name;
                }
                else if ((ref.isEmpty() == false) && (scope.kind != eScopeKind::Document))
                {
                    appendUnique(out.groupRefs[scopeKey(scope)], ref.toString());
                }
            }

            stack.append(scope);
        }
    }

    //!< Replaces every group reference with the elements that group declares.
    void expandGroups(Collected& data)
    {
        for (int pass = 0; pass < MAX_GROUP_DEPTH; ++pass)
        {
            bool changed = false;
            const QList<QString> scopes = data.groupRefs.keys();
            for (const QString& scope : scopes)
            {
                for (const QString& group : data.groupRefs.value(scope))
                {
                    // Taken by value: the insert below may move what the hash holds.
                    const QStringList members = data.children.value(QStringLiteral("g:") + group);
                    for (const QString& member : members)
                    {
                        QStringList& target = data.children[scope];
                        if (target.contains(member) == false)
                        {
                            target.append(member);
                            changed = true;
                        }
                    }

                    for (const QString& nested : data.groupRefs.value(QStringLiteral("g:") + group))
                    {
                        QStringList& target = data.groupRefs[scope];
                        if (target.contains(nested) == false)
                        {
                            target.append(nested);
                            changed = true;
                        }
                    }
                }
            }

            if (changed == false)
            {
                break;
            }
        }
    }

    //!< The elements a scope stands for: itself when it is an element, everyone written with it
    //!< when it is a named type. A group has been expanded away by now.
    QStringList ownersOf(const Collected& data, const QString& scopeKey)
    {
        if (scopeKey.startsWith(QLatin1StringView("e:")))
        {
            return QStringList { scopeKey.sliced(2) };
        }

        if (scopeKey.startsWith(QLatin1StringView("t:")))
        {
            return data.scopesOfType.value(scopeKey.sliced(2));
        }

        return QStringList();
    }
}

QString DocSchemaReader::deliveryDirectory()
{
    // Asking for the executable's directory before there is an application is an error Qt
    // reports rather than answers, and a headless caller has no delivered copy to find anyway.
    return (QCoreApplication::instance() != nullptr)
                ? QCoreApplication::applicationDirPath() + QStringLiteral("/schema")
                : QString();
}

QList<DocSchemaReader::Element> DocSchemaReader::read(const QByteArray& schema)
{
    QList<Element> result;
    if (schema.isEmpty())
    {
        return result;
    }

    Collected data;
    QXmlStreamReader xml(schema);
    collect(xml, data);
    if (xml.hasError() || data.root.isEmpty())
    {
        return result;
    }

    expandGroups(data);

    QHash<QString, QStringList> parents;
    parents.insert(data.root, QStringList());

    for (auto it = data.children.constBegin(); it != data.children.constEnd(); ++it)
    {
        const QStringList owners = ownersOf(data, it.key());
        if (owners.isEmpty())
        {
            continue;
        }

        for (const QString& child : it.value())
        {
            QStringList& known = parents[child];
            for (const QString& owner : owners)
            {
                appendUnique(known, owner);
            }
        }
    }

    result.reserve(parents.size());
    for (auto it = parents.constBegin(); it != parents.constEnd(); ++it)
    {
        QStringList sorted = it.value();
        std::sort(sorted.begin(), sorted.end());
        result.append(Element { it.key(), sorted });
    }

    std::sort(result.begin(), result.end()
            , [](const Element& left, const Element& right) { return left.name < right.name; });
    return result;
}

QList<DocSchemaReader::Element> DocSchemaReader::readFile(const QString& path)
{
    QFile file(path);
    return file.open(QIODevice::ReadOnly) ? read(file.readAll()) : QList<Element>();
}
