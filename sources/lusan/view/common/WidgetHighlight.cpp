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
 *  \file        lusan/view/common/WidgetHighlight.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, a short-lived accent on the control a navigation landed on.
 *
 ************************************************************************/

#include "lusan/view/common/WidgetHighlight.hpp"

#include <QScrollArea>
#include <QString>
#include <QTimer>
#include <QVariant>
#include <QWidget>

namespace
{
    //!< How long the accent stays before the control goes back to its own look.
    constexpr int HoldMs{ 2000 };

    //!< Carries the control's own style across the accent, so restoring is exact.
    const char* const PropertySavedStyle{ "lusanSavedStyleSheet" };

    /**
     * Amber reads as "look here" against both the light and the dark palette, and the tint is
     * translucent so the control keeps whatever background its theme gave it.
     */
    QString accentStyle(const QWidget& field)
    {
        // Scoped to the control's own class, otherwise the rule reaches its children as well.
        return QStringLiteral("%1 { border: 2px solid #f39c12; background-color: rgba(243, 156, 18, 40); }")
                    .arg(QString::fromLatin1(field.metaObject()->className()));
    }
}

void WidgetHighlight::reveal(QWidget* field)
{
    if (field == nullptr)
    {
        return;     // the page has no such control; the element selection is the whole reveal
    }

    // The detail forms sit in scroll areas, and an accent below the fold points at nothing.
    for (QWidget* parent = field->parentWidget(); parent != nullptr; parent = parent->parentWidget())
    {
        QScrollArea* area = qobject_cast<QScrollArea*>(parent);
        if (area != nullptr)
        {
            area->ensureWidgetVisible(field);
            break;
        }
    }

    field->setFocus(Qt::OtherFocusReason);

    // Only the first accent records the style to return to. A second jump onto a control that is
    // still tinted would otherwise save the tint and never take it off again.
    if (field->property(PropertySavedStyle).isValid() == false)
    {
        field->setProperty(PropertySavedStyle, field->styleSheet());
    }

    field->setStyleSheet(accentStyle(*field));

    // The control is the timer's context: closing the document mid-accent cancels it instead of
    // touching a widget that is on its way out.
    QTimer::singleShot(HoldMs, field, [field]()
    {
        field->setStyleSheet(field->property(PropertySavedStyle).toString());
        field->setProperty(PropertySavedStyle, QVariant());
    });
}
