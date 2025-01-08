#ifndef LUSAN_APPLICATION_SI_SIDATATYPE_HPP
#define LUSAN_APPLICATION_SI_SIDATATYPE_HPP
/************************************************************************
 *  This file is part of the Lusan project, an official component of the AREG SDK.
 *  Lusan is a graphical user interface (GUI) tool designed to support the development,
 *  debugging, and testing of applications built with the AREG Framework.
 *
 *  Lusan is available as free and open-source software under the MIT License,
 *  providing essential features for developers.
 *
 *  For detailed licensing terms, please refer to the LICENSE.txt file included
 *  with this distribution or contact us at info[at]aregtech.com.
 *
 *  \copyright   © 2023-2024 Aregtech UG. All rights reserved.
 *  \file        lusan/view/si/SIDataType.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/
#include <QScrollArea>

class SIDataTypeDetails;
class SIDataTypeList;
class SIDataTypeFieldDetails;
class SIDataTypeModel;

namespace Ui {
    class SIDataType;
}

class SIDataTypeWidget : public QWidget
{
    friend class SIDataType;

    Q_OBJECT

public:
    explicit SIDataTypeWidget(QWidget* parent);

private:
    Ui::SIDataType * ui;
};

class SIDataType : public QScrollArea
{
    Q_OBJECT

public:
    explicit SIDataType(SIDataTypeModel & model, QWidget *parent = nullptr);

    virtual ~SIDataType(void);

protected:
    
    virtual void closeEvent(QCloseEvent *event) override;

    virtual void hideEvent(QHideEvent *event) override;

    /**
     * \brief Triggered when the current cell is changed.
     * \param currentRow The current row index.
     * \param currentColumn The current column index.
     * \param previousRow The previous row index.
     * \param previousColumn The previous column index.
     */
    void onCurCellChanged(int currentRow, int currentColumn, int previousRow, int previousColumn);

    /**
     * \brief Triggered when the add button is clicked.
     */
    void onAddClicked(void);

    /**
     * \brief Triggered when the remove button is clicked.
     */
    void onRemoveClicked(void);

    /**
     * \brief Triggered when the name is changed.
     * \param newName The new name of the attribute.
     */
    void onNameChanged(const QString& newName);

    /**
     * \brief   Triggered when the deprecated flag is changed.
     * \param   isChecked   The flag to indicate if the constant is deprecated.
     **/
    void onDeprectedChecked(bool isChecked);

    /**
     * \brief   Triggered when the deprecation hint is changed.
     * \param   newText The new deprecation hint.
     **/
    void onDeprecateHintChanged(const QString& newText);

    /**
     * \brief   Triggered when the description is changed.
     **/
    void onDescriptionChanged(void);

private:

    void showEnumDetails(bool show);

    void showImportDetails(bool show);

    void showContainerDetails(bool show);
    
    /**
     * \brief Updates the data in the table.
     */
    void updateData(void);
    
    /**
     * \brief Updates the widgets.
     */
    void updateWidgets(void);
    
    /**
     * \brief Initializes the signals.
     */
    void setupSignals(void);
    
private:
    SIDataTypeDetails*      mDetails;
    SIDataTypeList*         mList;
    SIDataTypeFieldDetails* mFields;
    SIDataTypeWidget*       mWidget;
    Ui::SIDataType &        ui;
    SIDataTypeModel&        mModel;

    uint32_t                mCount;
};

#endif // LUSAN_APPLICATION_SI_SIDATATYPE_HPP
