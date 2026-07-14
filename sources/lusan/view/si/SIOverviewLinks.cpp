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
 *  \file        lusan/view/si/SIOverviewLinks.cpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface overview quick links.
 *
 ************************************************************************/
#include "lusan/view/si/SIOverviewLinks.hpp"

#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace
{
    QPushButton* makeLink(QWidget* parent, const QString& objectName, const QString& text, const QString& toolTip)
    {
        QPushButton* button = new QPushButton(text, parent);
        button->setObjectName(objectName);
        QFont font{ button->font() };
        font.setBold(true);
        font.setUnderline(true);
        button->setFont(font);
        button->setCursor(Qt::PointingHandCursor);
        button->setMouseTracking(true);
        button->setToolTip(toolTip);
        button->setFlat(true);
        return button;
    }
}

SIOverviewLinks::SIOverviewLinks(QWidget* parent)
    : QWidget       (parent)
    , mDataTypes    (nullptr)
    , mAttributes   (nullptr)
    , mMethods      (nullptr)
    , mConstants    (nullptr)
    , mIncludes     (nullptr)
{
    buildUi();
}

void SIOverviewLinks::buildUi()
{
    QVBoxLayout* root = new QVBoxLayout(this);
    root->setContentsMargins(0, 0, 0, 0);

    QGroupBox* group = new QGroupBox(tr("Quick Links:"), this);
    QFormLayout* form = new QFormLayout(group);
    form->setRowWrapPolicy(QFormLayout::DontWrapRows);
    form->setLabelAlignment(Qt::AlignLeft | Qt::AlignTop);

    mDataTypes  = makeLink(group, QStringLiteral("linkDataTypes") , tr("Data Types ..."), tr("Click to open Interface Data Types page"));
    mAttributes = makeLink(group, QStringLiteral("linkAttributes"), tr("Attributes ..."), tr("Click to open Interface Attributes page"));
    mMethods    = makeLink(group, QStringLiteral("linkMethods")   , tr("Methods ...")   , tr("Click to open Interface Method page"));
    mConstants  = makeLink(group, QStringLiteral("linkConstants") , tr("Constants ...") , tr("Click to open Interface available Constants"));
    mIncludes   = makeLink(group, QStringLiteral("linkIncludes")  , tr("Includes ...")  , tr("Click to open Interface additional Includes"));

    form->addRow(mDataTypes , new QLabel(tr("Open Service Interface Data Types Page ..."), group));
    form->addRow(mAttributes, new QLabel(tr("Open Service Interface Data Attributes Page ..."), group));
    form->addRow(mMethods   , new QLabel(tr("Open Service Interface Methods Page ..."), group));
    form->addRow(mConstants , new QLabel(tr("Open Service Interface Constants Page ..."), group));
    form->addRow(mIncludes  , new QLabel(tr("Open Service Interface Includes Page ..."), group));

    root->addWidget(group);
    root->addStretch(1);
}

QPushButton* SIOverviewLinks::linkDataTypes() const
{
    return mDataTypes;
}

QPushButton* SIOverviewLinks::linkAttributes() const
{
    return mAttributes;
}

QPushButton* SIOverviewLinks::linkMethods() const
{
    return mMethods;
}

QPushButton* SIOverviewLinks::linkConstants() const
{
    return mConstants;
}

QPushButton* SIOverviewLinks::linkIncludes() const
{
    return mIncludes;
}
