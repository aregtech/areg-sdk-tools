/************************************************************************
 *  This file is part of the Lusan project, an official component of the AREG SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the AREG Framework.
 *
 *  Lusan is available as free and open-source software under the MIT License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]areg.tech.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/view/common/TableCell.cpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, ComboBox for the table cell.
 *
 ************************************************************************/

#include "lusan/view/common/TableCell.hpp"
#include "lusan/app/LusanApplication.hpp"

#include <QComboBox>
#include <QStandardItemModel>
#include <QLineEdit>
#include <QStyleFactory>
#include <QTableWidget>
#include <QTableWidgetItem>

TableCell::TableCell(QWidget* parent, IETableHelper * tableHelper, bool waitEndEdit)
    : QStyledItemDelegate(parent)
    , mModels   ( )
    , mColumns  ( )
    , mParent   (parent)
    , mTable    (tableHelper)
    , mWaitEnd  (waitEndEdit)
    , mNewText  ( )
    , mSelIndex ( )
{
}

TableCell::TableCell(const QList<QAbstractItemModel*>& models, const QList<int>& columns, QWidget* parent, IETableHelper * tableHelper, bool waitEndEdit)
    : QStyledItemDelegate(parent)
    , mModels   (models)
    , mColumns  (columns)
    , mParent   (parent)
    , mTable    (tableHelper)
    , mWaitEnd  (waitEndEdit)
    , mNewText  ( )
    , mSelIndex ( )
{
    Q_ASSERT(models.size() == columns.size());
    for (QAbstractItemModel* model : models)
    {
        if (model != nullptr)
        {
            model->setParent(this);
        }
    }
}

inline bool TableCell::isValidColumn(int col) const
{
    return (col >= 0) && (col < mTable->getColumnCount());
}

bool TableCell::isComboWidget(int col) const
{
    if (isValidColumn(col))
    {
        for (int i = 0; i < static_cast<int>(mColumns.size()); ++i)
        {
            if (mColumns[i] == col)
            {
                return true;
            }
        }
    }

    return false;
}

QAbstractItemModel* TableCell::columnToModel(int col) const
{
    if (isValidColumn(col))
    {
        for (int i = 0; i < static_cast<int>(mColumns.size()); ++i)
        {
            if (mColumns[i] == col)
            {
                return static_cast<QAbstractItemModel*>(mModels[i]);
            }
        }
    }

    return nullptr;
}

QWidget* TableCell::createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& index) const
{
    QAbstractItemModel* model = columnToModel(index.column());
    mNewText.clear();
    mSelIndex = QModelIndex();
    if (model != nullptr)
    {
        QComboBox* combo = new QComboBox(parent);
        combo->setModel(model);
        combo->setProperty("index", index);
        connect(combo, &QComboBox::currentTextChanged, this, &TableCell::onComboTextChanged);
        
        return combo;
    }
    else if ( isValidColumn(index.column()) )
    {
        QLineEdit* lineEdit = new QLineEdit(parent);
        connect(lineEdit, &QLineEdit::textEdited, this, &TableCell::onEditorTextChanged);
        if (mWaitEnd)
        {
            connect(lineEdit, &QLineEdit::editingFinished, this, &TableCell::onEditorTextChangeFinished);
        }
        
        return lineEdit;
    }
    
    return nullptr;
}

void TableCell::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    if (isComboWidget(index.column()))
    {
        if (index.data(Qt::EditRole).isNull())
            qobject_cast<QComboBox*>(editor)->setCurrentIndex(-1);
        else
            qobject_cast<QComboBox*>(editor)->setCurrentText(index.model()->data(index, Qt::EditRole).toString());
    }
    else if (isValidColumn(index.column()))
    {
        QString text = mTable->getCellText(index);
        qobject_cast<QLineEdit*>(editor)->setText(text);
    }
}

void TableCell::updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if (editor != nullptr)
    {
        editor->setGeometry(option.rect);
        if (isComboWidget(index.column()))
        {
            static_cast<QComboBox*>(editor)->showPopup();
        }
    }
}

void TableCell::onComboTextChanged(const QString & newText)
{
    QWidget *editor = qobject_cast<QWidget *>(sender());
    if (editor != nullptr)
    {
        emit signalEditorDataChanged(editor->property("index").toModelIndex(), newText);
    }
}

void TableCell::onEditorTextChanged(const QString & newText)
{
    QWidget *editor = qobject_cast<QWidget *>(sender());
    if (editor != nullptr)
    {
        if (mWaitEnd == false)
        {
            emit signalEditorDataChanged(editor->property("index").toModelIndex(), newText);
        }
        else
        {
            mNewText = newText;
            mSelIndex = editor->property("index").toModelIndex();
        }
    }
}

void TableCell::onEditorTextChangeFinished(void)
{
    if (mWaitEnd && mSelIndex.isValid() && (mNewText.isEmpty() == false))
    {
        emit signalEditorDataChanged(mSelIndex, mNewText);
        mSelIndex = QModelIndex();
        mNewText.clear();
    }
}
