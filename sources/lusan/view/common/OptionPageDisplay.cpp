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
 *  \file        lusan/view/common/OptionPageDisplay.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, the display settings page.
 *
 ************************************************************************/

#include "lusan/view/common/OptionPageDisplay.hpp"

#include "lusan/app/LusanApplication.hpp"
#include "lusan/data/common/OptionsManager.hpp"

#include <QAbstractItemView>
#include <QApplication>
#include <QComboBox>
#include <QDialog>
#include <QFormLayout>
#include <QLabel>
#include <QVBoxLayout>

namespace
{
    //!< The units offered by the selector, in the order they are listed.
    constexpr NETimeUnits::eTimeUnit _units[]
    {
          NETimeUnits::eTimeUnit::UnitMicro
        , NETimeUnits::eTimeUnit::UnitMilli
        , NETimeUnits::eTimeUnit::UnitSecond
    };
}

OptionPageDisplay::OptionPageDisplay(QDialog* parent)
    : OptionPageBase(parent)
    , mTimeUnit     (nullptr)
{
    setupWidgets();
    refreshFromOptions();
}

void OptionPageDisplay::setupWidgets(void)
{
    setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(8);

    QLabel* title = new QLabel(tr("Display"), this);
    QFont titleFont{ title->font() };
    titleFont.setBold(true);
    title->setFont(titleFont);
    layout->addWidget(title, 0);

    QFormLayout* form = new QFormLayout();
    form->setContentsMargins(0, 6, 0, 0);
    form->setFieldGrowthPolicy(QFormLayout::FieldGrowthPolicy::FieldsStayAtSizeHint);
    form->setLabelAlignment(Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);

    mTimeUnit = new QComboBox(this);
    mTimeUnit->setToolTip(tr("The unit every duration and every elapsed time is written in."));
    for (NETimeUnits::eTimeUnit unit : _units)
    {
        mTimeUnit->addItem(NETimeUnits::unitName(unit), static_cast<int>(unit));
    }

    form->addRow(tr("Show measured times in:"), mTimeUnit);
    layout->addLayout(form);

    QLabel* hint = new QLabel(tr("The framework measures in microseconds. The other units are the same value written differently."), this);
    hint->setWordWrap(true);
    hint->setEnabled(false);
    layout->addWidget(hint, 0);
    layout->addStretch(1);

    connect(mTimeUnit, &QComboBox::currentIndexChanged, this, [this](int) {
        setDataModified(selectedUnit() != LusanApplication::getOptions().getTimeUnit());
        setCanSave(true);
    });
}

void OptionPageDisplay::refreshFromOptions(void)
{
    const int index{ mTimeUnit->findData(static_cast<int>(LusanApplication::getOptions().getTimeUnit())) };
    mTimeUnit->setCurrentIndex(index >= 0 ? index : 0);
    setDataModified(false);
    setCanSave(true);
}

NETimeUnits::eTimeUnit OptionPageDisplay::selectedUnit(void) const
{
    const int index{ mTimeUnit->currentIndex() };
    return index >= 0 ? static_cast<NETimeUnits::eTimeUnit>(mTimeUnit->itemData(index).toInt()) : NETimeUnits::DefaultUnit;
}

void OptionPageDisplay::applyChanges(void)
{
    if (isDataModified())
    {
        OptionsManager& options{ LusanApplication::getOptions() };
        options.setTimeUnit(selectedUnit());
        options.writeOptions();
        NETimeUnits::setUnit(selectedUnit());

        // Every table already holds the text of the previous unit, so the cells are asked again.
        const QWidgetList widgets{ QApplication::allWidgets() };
        for (QWidget* widget : widgets)
        {
            QAbstractItemView* view{ qobject_cast<QAbstractItemView *>(widget) };
            if (view != nullptr)
                view->viewport()->update();
        }
    }

    OptionPageBase::applyChanges();
}
