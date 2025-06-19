#ifndef LUSAN_VIEW_COMMON_OPTIONPAGELOGGING_HPP
#define LUSAN_VIEW_COMMON_OPTIONPAGELOGGING_HPP
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
 *  \file        lusan/view/common/OptionPageLogging.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Tamas Csillag, Artak Avetyan
 *  \brief       Lusan application, Log Settings page of options dialog.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/view/common/OptionPageBase.hpp"

#include "lusan/common/NELusanCommon.hpp"
#include "areg/component/NEService.hpp"

#include <QValidator>
#include <memory>

 /************************************************************************
  * Dependencies
  ************************************************************************/
namespace Ui {
    class OptionPageLoggingForm;
}
class ProjectSettings;
class QLineEdit;
class QPushButton;
class QTextEdit;

/**
 * \brief   User interface for configuring log settings in the Lusan application.
 **/
class OptionPageLogging : public OptionPageBase
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Internal types and constants
//////////////////////////////////////////////////////////////////////////
private:

    static const QString    _textNoChanges;         //!< Status text indicating no changes have been made.
    static const QString    _textDataChanged;       //!< Status text indicating that data has been changed and needs to be tested before saving.
    static const QString    _textTestInProgress;    //!< Status text indicating that a log collector service connection test is in progress.
    static const QString    _textTestInterrupted;   //!< Status text indicating that an ongoing connection test has been interrupted due to data changes.
    static const QString    _textServiceConnected;  //!< Status text indicating successful connection to the log collector service.
    static const QString    _textTestSucceeded;     //!< Status text indicating that the log collector service connection test was successful, along with receiving communication message.
    static const QString    _textConnectionFailed;  //!< Status text indicating that the connection trigger to the log collector service failed.
    static const QString    _textTestFailed;        //!< Status text indicating that the log collector service connection test failed.
    static const QString    _textTestCanceled;      //!< Status text indicating that the connection to the log collector service was canceled.

//////////////////////////////////////////////////////////////////////////
// Constructors / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit OptionPageLogging(ProjectSettings *parent);
    explicit OptionPageLogging(ProjectSettings *parent, const QString& address, uint16_t port, const QString &logFile, const QString &logLocation);
    ~OptionPageLogging() override;

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Call when the option should apply the changes.
     **/
    virtual void applyChanges(void) override;

    /**
     * \brief   Call when the option page is closing.
     * \param   OKpressed   If true, the user pressed OK button, otherwise Cancel button.
     **/
    virtual void closingOptions(bool OKpressed) override;
    
    /**
     * \brief   Triggered, letting option page object to display a warning message.
     **/
    virtual void warnMessage(void) override;
    
//////////////////////////////////////////////////////////////////////////
// Operations and attributes
//////////////////////////////////////////////////////////////////////////
public:

    /**
     * \brief   Sets the log settings data.
     * \param   address        The IP address of the log collector service.
     * \param   port           The port number of the log collector service.
     * \param   logFile        The name of the log file.
     * \param   logLocation    The directory where the log file is stored.
     **/
    void setData(const QString & address, uint16_t port, const QString & logFile, const QString & logLocation);

//////////////////////////////////////////////////////////////////////////
// Slots
//////////////////////////////////////////////////////////////////////////
private slots:

    /**
     * \brief   Slot triggered when the "Browse" button is clicked to select a log directory.
     **/
    void onBrowseButtonClicked();

    /**
     * \brief   Slot triggered when the "Test" button is clicked to test the log collector service connection.
     * \param   checked     The flag indicating if the button is checked.
     **/
    void onTestButtonClicked(bool checked);

    /**
     * \brief   Slot triggered when data in the log settings is changed.
     **/
    void onDataChanged();

    /**
     * \brief   Slot, triggered when the log location field is updated.
     **/
    void onLogLocationChanged(void);

    /**
     * \brief   Slot, triggered when the log file name field is updated.
     **/
    void onLogFileNameChanged(void);

    /**
     * \brief   Slot triggered when the log service connection status changes.
     * \param   isConnected     Indicates if the connection to the log service is successful.
     * \param   address         The IP address of the log service.
     * \param   port            The port number of the log service.
     **/
    void onLogServiceConnected(bool isConnected, const std::string& address, uint16_t port);

    /**
     * \brief   Slot triggered when the log source instance message is received.
     * \param   instances       A vector of connected service instances.
     **/
    void onLogInstancesConnected(const std::vector< NEService::sServiceConnectedInstance >& instances);
    
//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:

    /**
     * \brief   Initializes the dialog settings.
     **/
    void setupDialog();

    /**
     * \brief   Connects the signals and slots for the log settings.
     **/
    void connectSignals() const;

    /**
     * \brief   Saves the log settings data.
     **/
    void saveData() const;

    /**
     * \brief   Returns value of log location field.
     **/
    inline QString getLogLocation(void) const;
    
    /**
     * \brief   Returns value of log file name field.
     **/
    inline QString getLogFileName(void) const;
    
    /**
     * \brief   Returns value of log collector service IP-address field.
     **/
    inline QString getServiceAddress(void) const;
    
    /**
     * \brief   Returns value of log collector service port number field.
     **/
    inline uint16_t getServicePort(void) const;

/************************************************************************
 * Inline methods
 ************************************************************************/

    //!< Returns the widget for the log location.
    inline QLineEdit* textLogLocation(void) const;
    
    //!< Returns the widget for the log file name.
    inline QLineEdit* textLogFileName(void) const;

    //!< Returns the widget for the IP address or host name input to set the address of the log colletor service.
    inline QLineEdit* textIpAddress(void) const;

    //<! Returns the widget for the port number input.
    inline QLineEdit* textPortNumber(void) const;

    //<! Returns the widget for the connection status text.
    inline QTextEdit* textConnectionStatus(void) const;

    //<! Returns the button for browsing directories.
    inline QPushButton* buttonBrowseDirs(void) const;

    //<! Returns the button for testing the connection to the log collector service.
    inline QPushButton* buttonTestConnection(void) const;

//////////////////////////////////////////////////////////////////////////
// Hidden member variables
//////////////////////////////////////////////////////////////////////////
private:
    std::unique_ptr<Ui::OptionPageLoggingForm>    ui;             //!< The user interface object.
    QRegularExpressionValidator             mPortValidator; //!< The validator for the port number input.
    bool                                    mTestTriggered; //!< Flag indicating if the test connection is triggered.
    QString                                 mAddress;       //!< The address of the log collector service.
    uint16_t                                mPort;          //!< The port number of the log collector service.
    QString                                 mLogFileName;   //!< The name of the log file.
    QString                                 mLogLocation;   //!< The directory where the log file is stored.
    QMetaObject::Connection                 mTestConnect;   //<! Connection for the log collector service connection test.
    QMetaObject::Connection                 mTestMessage;   //<! Connection for the log collector service message test.

//////////////////////////////////////////////////////////////////////////
// Forbidden calls.
//////////////////////////////////////////////////////////////////////////
private:
    OptionPageLogging(void) = delete; // Disable default constructor
    DECLARE_NOCOPY_NOMOVE(OptionPageLogging);
};

#endif // LUSAN_VIEW_COMMON_OPTIONPAGELOGGING_HPP
