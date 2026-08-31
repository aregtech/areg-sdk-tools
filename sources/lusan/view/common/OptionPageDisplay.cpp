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
#include "lusan/view/common/LogRowsPreview.hpp"
#include "lusan/view/log/LogViewerBase.hpp"

#include <QAbstractItemView>
#include <QApplication>
#include <QComboBox>
#include <QDialog>
#include <QFormLayout>
#include <QFrame>
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

    //!< The colour sets offered by the selector, in the order they are listed.
    constexpr NELogPalette::eLogPalette _palettes[]
    {
          NELogPalette::eLogPalette::PaletteLadder
        , NELogPalette::eLogPalette::PaletteQuiet
        , NELogPalette::eLogPalette::PaletteClassic
    };

    //!< The row heights offered by the selector. How many rows fit on screen is what makes
    //!< a burst or a gap in the log visible without scrolling.
    const struct { int height; const char* name; } _rowHeights[]
    {
          { 21, QT_TRANSLATE_NOOP("OptionPageDisplay", "Dense") }
        , { 25, QT_TRANSLATE_NOOP("OptionPageDisplay", "Regular") }
        , { 34, QT_TRANSLATE_NOOP("OptionPageDisplay", "Relaxed") }
    };
}

OptionPageDisplay::OptionPageDisplay(QDialog* parent)
    : OptionPageBase(parent)
    , mTimeUnit     (nullptr)
    , mLogPalette   (nullptr)
    , mLogRowHeight (nullptr)
    , mPaletteHint  (nullptr)
    , mPreview      (nullptr)
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

    QFrame* separator = new QFrame(this);
    separator->setFrameShape(QFrame::Shape::HLine);
    separator->setFrameShadow(QFrame::Shadow::Sunken);
    layout->addSpacing(6);
    layout->addWidget(separator, 0);

    QLabel* logTitle = new QLabel(tr("Log rows"), this);
    logTitle->setFont(titleFont);
    layout->addWidget(logTitle, 0);

    QFormLayout* logForm = new QFormLayout();
    logForm->setContentsMargins(0, 6, 0, 0);
    logForm->setFieldGrowthPolicy(QFormLayout::FieldGrowthPolicy::FieldsStayAtSizeHint);
    logForm->setLabelAlignment(Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);

    mLogPalette = new QComboBox(this);
    mLogPalette->setToolTip(tr("The colours the message text of a log row is drawn with. The rail on the left edge keeps its colour in every set."));
    for (NELogPalette::eLogPalette entry : _palettes)
    {
        mLogPalette->addItem(NELogPalette::paletteName(entry), static_cast<int>(entry));
    }

    mLogRowHeight = new QComboBox(this);
    mLogRowHeight->setToolTip(tr("The height of one row. A shorter row puts more of the log on screen at once."));
    for (const auto& entry : _rowHeights)
    {
        mLogRowHeight->addItem( tr("%1 (%2 px)").arg(tr(entry.name)).arg(entry.height), entry.height);
    }

    logForm->addRow(tr("Colours:"), mLogPalette);
    logForm->addRow(tr("Row height:"), mLogRowHeight);
    layout->addLayout(logForm);

    mPaletteHint = new QLabel(this);
    mPaletteHint->setWordWrap(true);
    mPaletteHint->setEnabled(false);
    layout->addWidget(mPaletteHint, 0);

    mPreview = new LogRowsPreview(this);
    mPreview->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
    layout->addSpacing(4);
    layout->addWidget(mPreview, 0);
    layout->addStretch(1);

    connect(mTimeUnit, &QComboBox::currentIndexChanged, this, [this](int) {
        updateModified();
    });
    connect(mLogPalette, &QComboBox::currentIndexChanged, this, [this](int) {
        refreshPreview();
        updateModified();
    });
    connect(mLogRowHeight, &QComboBox::currentIndexChanged, this, [this](int) {
        refreshPreview();
        updateModified();
    });
}

void OptionPageDisplay::refreshPreview(void)
{
    mPaletteHint->setText(NELogPalette::paletteHint(selectedPalette()));
    mPreview->setSample(selectedPalette(), selectedRowHeight());
}

void OptionPageDisplay::updateModified(void)
{
    const OptionsManager& options{ LusanApplication::getOptions() };
    const bool changed{ (selectedUnit() != options.getTimeUnit())
                        || (selectedPalette() != options.getLogPalette())
                        || (selectedRowHeight() != options.getLogRowHeight()) };
    setDataModified(changed);
    setCanSave(true);
}

void OptionPageDisplay::refreshFromOptions(void)
{
    const OptionsManager& options{ LusanApplication::getOptions() };

    const int unit{ mTimeUnit->findData(static_cast<int>(options.getTimeUnit())) };
    mTimeUnit->setCurrentIndex(unit >= 0 ? unit : 0);

    const int palette{ mLogPalette->findData(static_cast<int>(options.getLogPalette())) };
    mLogPalette->setCurrentIndex(palette >= 0 ? palette : 0);

    // A height that is not one of the offered ones is still shown, so a value edited by
    // hand in the settings file is not silently replaced.
    int height{ mLogRowHeight->findData(options.getLogRowHeight()) };
    if (height < 0)
    {
        mLogRowHeight->addItem(tr("Custom (%1 px)").arg(options.getLogRowHeight()), options.getLogRowHeight());
        height = mLogRowHeight->count() - 1;
    }

    mLogRowHeight->setCurrentIndex(height);

    refreshPreview();
    setDataModified(false);
    setCanSave(true);
}

NETimeUnits::eTimeUnit OptionPageDisplay::selectedUnit(void) const
{
    const int index{ mTimeUnit->currentIndex() };
    return index >= 0 ? static_cast<NETimeUnits::eTimeUnit>(mTimeUnit->itemData(index).toInt()) : NETimeUnits::DefaultUnit;
}

NELogPalette::eLogPalette OptionPageDisplay::selectedPalette(void) const
{
    const int index{ mLogPalette->currentIndex() };
    return index >= 0 ? static_cast<NELogPalette::eLogPalette>(mLogPalette->itemData(index).toInt()) : NELogPalette::DefaultPalette;
}

int OptionPageDisplay::selectedRowHeight(void) const
{
    const int index{ mLogRowHeight->currentIndex() };
    return index >= 0 ? mLogRowHeight->itemData(index).toInt() : OptionsManager::LogRowHeightDefault;
}

void OptionPageDisplay::applyChanges(void)
{
    if (isDataModified())
    {
        OptionsManager& options{ LusanApplication::getOptions() };
        options.setTimeUnit(selectedUnit());
        options.setLogPalette(selectedPalette());
        options.setLogRowHeight(selectedRowHeight());
        options.writeOptions();

        NETimeUnits::setUnit(selectedUnit());
        NELogPalette::setPalette(selectedPalette());
        LogViewerBase::refreshRowHeights();

        // Every table already holds the text and the colours of the previous settings, so
        // the cells are asked again.
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
