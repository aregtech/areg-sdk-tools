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
#include <QScrollArea>
#include <QIntValidator>

class SIOverviewDetails;
class SIOverviewLinks;
class SIOverviewParamDetails;
class SIOverviewModel;

namespace Ui {
    class SIOverview;
}

class SIOverviewWidget : public QWidget
{
    friend class SIOverview;

    Q_OBJECT

public:
    explicit SIOverviewWidget(QWidget* parent);

private:
    Ui::SIOverview* ui;
};

class SIOverview : public QScrollArea
{
    Q_OBJECT

public:
    explicit SIOverview(SIOverviewModel & model, QWidget* parent = nullptr);

    virtual ~SIOverview(void);

protected:

    void onCheckedPublic(bool isChecked);

    void onCheckedPrivate(bool isChecked);

    void onCheckedInternet(bool isChecked);

    void onDeprecatedChecked(bool isChecked);

    void onDescriptionChanged(void);

    void onDeprecateHintChanged(const QString& newText);

    void onMajorChanged(const QString& major);

    void onMinorChanged(const QString& minor);

    void onPatchChanged(const QString& patch);

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

private:
    SIOverviewModel&    mModel;
    SIOverviewDetails*  mDetails;
    SIOverviewLinks*    mLinks;
    SIOverviewWidget*   mWidget;
    Ui::SIOverview&     ui;
    QIntValidator       mVersionValidator;
};

#endif // LUSAN_APPLICATION_SI_SIOVERVIEW_HPP
