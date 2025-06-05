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
#include <QValidator>
#include <memory>

namespace Ui {
class LogSettingsForm;
}
class Component;


class LogSettings : public QWidget
{
    Q_OBJECT

public:
    explicit LogSettings(QWidget *parent = nullptr);
    ~LogSettings() override;

    LogSettings(LogSettings const&) = delete;
    LogSettings& operator= (LogSettings const&) = delete;
    LogSettings(LogSettings &&) noexcept = delete;
    LogSettings& operator= (LogSettings &&) noexcept = delete;

    void applyChanges();

private slots:
    void logDirBrowseButtonClicked();
    void testEndpointButtonClicked();
    void endpointChanged();

private:
    void setupDialog();
    void connectSignals() const;
    void loadData();
    void saveData() const;

    std::unique_ptr<Ui::LogSettingsForm> mUi;                 //!< The user interface object.
    std::unique_ptr<QValidator> mIpValidator;
    std::unique_ptr<QValidator> mPortValidator;
    bool mIsConnectedToConfiguredEndpoint;
};

#endif // LUSAN_VIEW_COMMON_LOGSETTINGS_HPP
