#ifndef LUSAN_APPLICATION_SI_SIOVERVIEW_HPP
#define LUSAN_APPLICATION_SI_SIOVERVIEW_HPP
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
 *  \file        lusan/view/si/SIOverview.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, Service Interface Overview section.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include <QScrollArea>
#include <QIntValidator>
#include "lusan/view/common/IEDataTypeConsumer.hpp"

/************************************************************************
 * Dependencies
 ************************************************************************/
class SIOverviewDetails;
class SIOverviewLinks;
class SIOverviewParamDetails;
class SIOverviewModel;

namespace Ui {
    class SIOverview;
}

/************************************************************************
 * \brief   The helper widget.
 ************************************************************************/
class SIOverviewWidget : public QWidget
{
    friend class SIOverview;

    Q_OBJECT

public:
    explicit SIOverviewWidget(QWidget* parent);

private:
    Ui::SIOverview* ui;
};

/**
 * \brief   The widget to display the overview details of the service interface.
 *          The widget contains the information about the service interface, such as
 *          name, version, description, and other details.
 **/
class SIOverview : public QScrollArea
                 , public IEDataTypeConsumer
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SIOverview(SIOverviewModel & model, QWidget* parent = nullptr);

    virtual ~SIOverview(void);

//////////////////////////////////////////////////////////////////////////
// Attributes and operations
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Sets the service interface name.
     **/
    void setServiceInterfaceName(const QString & siName);

signals:

    /**
     * \brief   The signal is triggered when the page link is clicked.
     * \param   page    The index of the page.
     **/
    void signalPageLinkClicked(int page);

//////////////////////////////////////////////////////////////////////////
// Slots
//////////////////////////////////////////////////////////////////////////
protected:

    /**
     * \brief   The slot is triggered when service interface category is set as Public.
     * \param   isChecked   The flag indicating whether the radio button is checked or not.
     **/
    void onCheckedPublic(bool isChecked);

    /**
     * \brief   The slot is triggered when service interface category is set as Private.
     * \param   isChecked   The flag indicating whether the radio button is checked or not.
     **/
    void onCheckedPrivate(bool isChecked);

    /**
     * \brief   The slot is triggered when service interface category is set as Internet.
     * \param   isChecked   The flag indicating whether the radio button is checked or not.
     **/
    void onCheckedInternet(bool isChecked);


    void onDeprecatedChecked(bool isChecked);

    /**
     * \brief   The slot is triggered when the service interface description is changed.
     **/
    void onDescriptionChanged(void);

    /**
     * \brief   The slot is triggered when the deprecation hint is changed.
     * \param   newText     The new text of the deprecation hint.
     **/
    void onDeprecateHintChanged(const QString& newText);

    /**
     * \brief   The slot is triggered when the service interface major version is changed.
     * \param   major   The new major version.
     **/
    void onMajorChanged(const QString& major);

    /**
     * \brief   The slot is triggered when the service interface minor version is changed.
     * \param   minor   The new minor version.
     **/
    void onMinorChanged(const QString& minor);

    /**
     * \brief   The slot is triggered when the service interface patch version is changed.
     * \param   patch   The new patch version.
     **/
    void onPatchChanged(const QString& patch);

    /**
     * \brief   The slot is triggered when the link to constants tabbed page is clicked.
     * \param   checked The flag indicating whether the link is checked or not.
     **/
    void onLinkConstantsClicked(bool checked);

    /**
     * \brief   The slot is triggered when the link to data types tabbed page is clicked.
     * \param   checked The flag indicating whether the link is checked or not.
     **/
    void onLinkDataTypesClicked(bool checked);

    /**
     * \brief   The slot is triggered when the link to includes tabbed page is clicked.
     * \param   checked The flag indicating whether the link is checked or not.
     **/
    void onLinkIncludesClicked(bool checked);

    /**
     * \brief   The slot is triggered when the link to methods tabbed page is clicked.
     * \param   checked The flag indicating whether the link is checked or not.
     **/
    void onLinkMethodsClicked(bool checked);

    /**
     * \brief   The slot is triggered when the link to attributes tabbed page is clicked.
     * \param   checked The flag indicating whether the link is checked or not.
     **/
    void onLinkTopicsClicked(bool checked);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    /**
     * \brief   Initializes the SIConstant object.
     **/
    void updateWidgets(void);

    /**
     * \brief   Initializes the SIInclude object.
     **/
    void updateData(void);

    /**
     * \brief   Initializes the signals.
     **/
    void setupSignals(void);

//////////////////////////////////////////////////////////////////////////
// Hidden member variables
//////////////////////////////////////////////////////////////////////////
private:
    SIOverviewModel&    mModel;     //!< The model of the service interface overview.
    SIOverviewDetails*  mDetails;   //!< The details of the service interface overview.
    SIOverviewLinks*    mLinks;     //!< The links of the service interface overview.
    SIOverviewWidget*   mWidget;    //!< The widget of the service interface overview.
    Ui::SIOverview&     ui;         //!< The user interface of the service interface overview.
    QIntValidator       mVersionValidator; //!< The validator to validate the version number.
};

#endif // LUSAN_APPLICATION_SI_SIOVERVIEW_HPP
