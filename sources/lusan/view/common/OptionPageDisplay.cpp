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
#include <QCheckBox>
#include <QComboBox>
#include <QDialog>
#include <QFontDatabase>
#include <QFormLayout>
#include <QFrame>
#include <QLabel>
#include <QSpinBox>
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

    //!< The timestamp shapes offered by the selector, in the order they are listed.
    constexpr NETimeUnits::eTimeStamp _stamps[]
    {
          NETimeUnits::eTimeStamp::StampTime
        , NETimeUnits::eTimeStamp::StampTimeMicro
        , NETimeUnits::eTimeStamp::StampDateTime
        , NETimeUnits::eTimeStamp::StampElapsed
        , NETimeUnits::eTimeStamp::StampDelta
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
    , mTimeStamp    (nullptr)
    , mLogPalette   (nullptr)
    , mLogRowHeight (nullptr)
    , mWordWrap     (nullptr)
    , mWrapLines    (nullptr)
    , mTimeSample   (nullptr)
    , mPaletteHint  (nullptr)
    , mPreview      (nullptr)
{
    setupWidgets();
    refreshFromOptions();
}

QFormLayout* OptionPageDisplay::addSection(const QString& title)
{
    QVBoxLayout* stack{ static_cast<QVBoxLayout *>(layout()) };

    QLabel* caption = new QLabel(title, this);
    QFont face{ caption->font() };
    face.setBold(true);
    caption->setFont(face);

    QFrame* rule = new QFrame(this);
    rule->setFrameShape(QFrame::Shape::HLine);
    rule->setFrameShadow(QFrame::Shadow::Plain);
    rule->setLineWidth(1);

    stack->addSpacing(6);
    stack->addWidget(caption, 0);
    stack->addWidget(rule, 0);

    QFormLayout* form = new QFormLayout();
    form->setContentsMargins(0, 6, 0, 0);
    form->setFieldGrowthPolicy(QFormLayout::FieldGrowthPolicy::FieldsStayAtSizeHint);
    form->setLabelAlignment(Qt::AlignmentFlag::AlignRight | Qt::AlignmentFlag::AlignVCenter);
    stack->addLayout(form);

    return form;
}

void OptionPageDisplay::addHint(const QString& text)
{
    QLabel* hint = new QLabel(text, this);
    hint->setWordWrap(true);
    hint->setEnabled(false);
    static_cast<QVBoxLayout *>(layout())->addWidget(hint, 0);
}

void OptionPageDisplay::setupWidgets(void)
{
    setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Expanding);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(10, 10, 10, 10);
    layout->setSpacing(6);

    QFormLayout* timeForm{ addSection(tr("Time")) };

    mTimeUnit = new QComboBox(this);
    mTimeUnit->setToolTip(tr("The unit every duration and every elapsed time is written in."));
    for (NETimeUnits::eTimeUnit unit : _units)
    {
        mTimeUnit->addItem(NETimeUnits::unitName(unit), static_cast<int>(unit));
    }

    mTimeStamp = new QComboBox(this);
    mTimeStamp->setToolTip(tr("The shape a log row writes its time in. The whole date and the microseconds stay in the tool tip of the cell."));
    for (NETimeUnits::eTimeStamp stamp : _stamps)
    {
        mTimeStamp->addItem(NETimeUnits::stampName(stamp), static_cast<int>(stamp));
    }

    timeForm->addRow(tr("Durations and elapsed times:"), mTimeUnit);
    timeForm->addRow(tr("Timestamps:"), mTimeStamp);

    mTimeSample = new QLabel(this);
    mTimeSample->setTextInteractionFlags(Qt::TextInteractionFlag::TextSelectableByMouse);
    QFont sampleFace{ QFontDatabase::systemFont(QFontDatabase::SystemFont::FixedFont) };
    sampleFace.setPointSizeF(font().pointSizeF());
    mTimeSample->setFont(sampleFace);
    timeForm->addRow(tr("Sample:"), mTimeSample);

    addHint(tr("The framework measures in microseconds. A log table writes the day only on the row that opens it, and the whole reading is in the tool tip of the cell."));

    QFormLayout* logForm{ addSection(tr("Log rows")) };

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

    mWordWrap = new QCheckBox(tr("Wrap a long message over several lines"), this);
    mWordWrap->setToolTip(tr("A message wider than its column is written over several lines instead of being cut."));

    mWrapLines = new QSpinBox(this);
    mWrapLines->setRange(OptionsManager::LogWrapLinesMin, OptionsManager::LogWrapLinesMax);
    mWrapLines->setToolTip(tr("The most lines one row may take. What does not fit is cut, and the whole message stays in the tool tip."));

    logForm->addRow(tr("Colours:"), mLogPalette);
    logForm->addRow(tr("Row height:"), mLogRowHeight);
    logForm->addRow(QString(), mWordWrap);
    logForm->addRow(tr("Lines at most:"), mWrapLines);

    mPreview = new LogRowsPreview(this);
    mPreview->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
    layout->addSpacing(4);
    layout->addWidget(mPreview, 0);

    mPaletteHint = new QLabel(this);
    mPaletteHint->setWordWrap(true);
    mPaletteHint->setEnabled(false);
    layout->addWidget(mPaletteHint, 0);
    layout->addStretch(1);

    connect(mTimeUnit, &QComboBox::currentIndexChanged, this, [this](int) {
        refreshTimeSample();
        updateModified();
    });
    connect(mTimeStamp, &QComboBox::currentIndexChanged, this, [this](int) {
        refreshTimeSample();
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
    connect(mWordWrap, &QCheckBox::toggled, this, [this](bool checked) {
        mWrapLines->setEnabled(checked);
        updateModified();
    });
    connect(mWrapLines, &QSpinBox::valueChanged, this, [this](int) {
        updateModified();
    });
}

void OptionPageDisplay::refreshPreview(void)
{
    mPaletteHint->setText(NELogPalette::paletteHint(selectedPalette()));
    mPreview->setSample(selectedPalette(), selectedRowHeight());
}

void OptionPageDisplay::refreshTimeSample(void)
{
    // The sample is written with the chosen settings and not with the ones in force, so the
    // line answers what the table will look like before the page is applied.
    const NETimeUnits::eTimeUnit keep{ NETimeUnits::unit() };
    NETimeUnits::setUnit(selectedUnit());
    const QString sample{ tr("%1     scope call %2").arg(NETimeUnits::stampSample(selectedStamp()))
                                                    .arg(NETimeUnits::duration(1240)) };
    NETimeUnits::setUnit(keep);

    mTimeSample->setText(sample);
}

void OptionPageDisplay::updateModified(void)
{
    const OptionsManager& options{ LusanApplication::getOptions() };
    const bool changed{ (selectedUnit() != options.getTimeUnit())
                        || (selectedStamp() != options.getTimeStamp())
                        || (selectedPalette() != options.getLogPalette())
                        || (selectedRowHeight() != options.getLogRowHeight())
                        || (mWordWrap->isChecked() != options.isLogWordWrap())
                        || (mWrapLines->value() != options.getLogWrapLines()) };
    setDataModified(changed);
    setCanSave(true);
}

void OptionPageDisplay::refreshFromOptions(void)
{
    const OptionsManager& options{ LusanApplication::getOptions() };

    const int unit{ mTimeUnit->findData(static_cast<int>(options.getTimeUnit())) };
    mTimeUnit->setCurrentIndex(unit >= 0 ? unit : 0);

    const int stamp{ mTimeStamp->findData(static_cast<int>(options.getTimeStamp())) };
    mTimeStamp->setCurrentIndex(stamp >= 0 ? stamp : 0);

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

    mWordWrap->setChecked(options.isLogWordWrap());
    mWrapLines->setValue(options.getLogWrapLines());
    mWrapLines->setEnabled(options.isLogWordWrap());

    refreshTimeSample();
    refreshPreview();
    setDataModified(false);
    setCanSave(true);
}

NETimeUnits::eTimeUnit OptionPageDisplay::selectedUnit(void) const
{
    const int index{ mTimeUnit->currentIndex() };
    return index >= 0 ? static_cast<NETimeUnits::eTimeUnit>(mTimeUnit->itemData(index).toInt()) : NETimeUnits::DefaultUnit;
}

NETimeUnits::eTimeStamp OptionPageDisplay::selectedStamp(void) const
{
    const int index{ mTimeStamp->currentIndex() };
    return index >= 0 ? static_cast<NETimeUnits::eTimeStamp>(mTimeStamp->itemData(index).toInt()) : NETimeUnits::DefaultStamp;
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
        options.setTimeStamp(selectedStamp());
        options.setLogPalette(selectedPalette());
        options.setLogRowHeight(selectedRowHeight());
        options.setLogWordWrap(mWordWrap->isChecked());
        options.setLogWrapLines(mWrapLines->value());
        options.writeOptions();

        NETimeUnits::setUnit(selectedUnit());
        NETimeUnits::setStamp(selectedStamp());
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
