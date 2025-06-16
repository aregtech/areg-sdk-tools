#ifndef LUSAN_VIEW_COMMON_LOGSETTINGS_HPP
#define LUSAN_VIEW_COMMON_LOGSETTINGS_HPP
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
 *  \file        lusan/view/common/ProjectSettings.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Tamas Csillag
 *  \brief       Lusan application, options dialog.
 *
 ************************************************************************/

#include <QWidget>

#include "lusan/common/NELusanCommon.hpp"
#include "areg/component/NEService.hpp"

#include <QValidator>
#include <memory>

namespace Ui {
class LogSettingsForm;
}
class ProjectSettings;
class QLineEdit;
class QPushButton;


class LogSettings : public QWidget
{
    Q_OBJECT

public:
    explicit LogSettings(ProjectSettings *parent);
    explicit LogSettings(ProjectSettings *parent, const QString& address, uint16_t port, const QString &logFile, const QString &logLocation);
    ~LogSettings() override;

    void applyChanges();
    
    void setData(const QString & address, uint16_t port, const QString & logFile, const QString & logLocation);

private slots:
    void onBrowseButtonClicked();
    void onTestButtonClicked(bool checked);
    void onDataChanged();
    
    void onLogServiceConnected(bool isConnected, const std::string& address, uint16_t port);
    
    void onLogInstancesConnected(const std::vector< NEService::sServiceConnectedInstance >& instances);
    
private:
    void setupDialog();
    void connectSignals() const;
    void saveData() const;
    
    inline QLineEdit* textLogLocation(void) const;
    
    inline QLineEdit* textLogFileName(void) const;
    
    inline QLineEdit* textIpAddress(void) const;
    
    inline QLineEdit* textPortNumber(void) const;
    
    inline QLineEdit* textConnectionStatus(void) const;
    
    inline QPushButton* buttonBrowseDirs(void) const;
    
    inline QPushButton* buttonTestConnection(void) const;

private:
    std::unique_ptr<Ui::LogSettingsForm>    ui;                 //!< The user interface object.
    QRegularExpressionValidator             mPortValidator;
    bool                                    mTestTriggered;
    bool                                    mCanSave;
    QString                                 mAddress;
    uint16_t                                mPort;
    QString                                 mLogFileName;
    QString                                 mLogLocation;

private:
    LogSettings(void) = delete; // Disable default constructor
    LogSettings(LogSettings const&) = delete;
    LogSettings& operator= (LogSettings const&) = delete;
    LogSettings(LogSettings&&) noexcept = delete;
    LogSettings& operator= (LogSettings&&) noexcept = delete;

};

#endif // LUSAN_VIEW_COMMON_LOGSETTINGS_HPP
