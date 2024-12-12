#ifndef LUSAN_APPLICATION_MAIN_MDICHILD_HPP
#define LUSAN_APPLICATION_MAIN_MDICHILD_HPP
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
 *  \file        lusan/application/main/MdiChild.hpp
 *  \ingroup     Lusan - GUI Tool for AREG SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application Multi-document interface (MDI) child window.
 *
 ************************************************************************/

#include <QTextEdit>

class MdiChild : public QTextEdit
{
    Q_OBJECT

public:
    MdiChild();

    void newFile();
    bool loadFile(const QString& fileName);
    bool save();
    bool saveAs();
    bool saveFile(const QString& fileName);
    QString userFriendlyCurrentFile();
    QString currentFile() { return curFile; }

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void documentWasModified();

private:
    bool maybeSave();
    void setCurrentFile(const QString& fileName);
    QString strippedName(const QString& fullFileName);

    QString curFile;
    bool isUntitled;
};

#endif // LUSAN_APPLICATION_MAIN_MDICHILD_HPP
