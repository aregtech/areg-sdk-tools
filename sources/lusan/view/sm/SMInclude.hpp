#ifndef LUSAN_VIEW_SM_SMINCLUDE_HPP
#define LUSAN_VIEW_SM_SMINCLUDE_HPP
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
 *  \file        lusan/view/sm/SMInclude.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, FSM Includes page.
 *
 ************************************************************************/

/************************************************************************
 * Includes
 ************************************************************************/
#include "lusan/view/common/IncludePage.hpp"

#include <QStringList>
#include <cstdint>

/************************************************************************
 * Dependencies
 ************************************************************************/
class IncludeEntry;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QPushButton;
class SMIncludeModel;

/**
 * \brief   The FSM Includes page: the shared \ref IncludePage plus what only a state machine
 *          does with an included file. A `.fsml` row is an imported machine, so it gets its own
 *          editor -- an alias, a pinned version and a resolution status -- and the safety checks
 *          that keep an import from closing a cycle or nesting too deep.
 **/
class SMInclude : public IncludePage
{
    Q_OBJECT

//////////////////////////////////////////////////////////////////////////
// Constructor / Destructor
//////////////////////////////////////////////////////////////////////////
public:
    explicit SMInclude(SMIncludeModel& model, QWidget* parent = nullptr);

    virtual ~SMInclude(void) = default;

//////////////////////////////////////////////////////////////////////////
// Shared entry points
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Opens the machine browse dialog and runs the add-time safety checks on the pick.
     * \return  The picked absolute file path, or an empty string when nothing was picked or the
     *          candidate was refused.
     **/
    static QString browseForMachine(SMIncludeModel& model, QWidget* parent);

    /**
     * \brief   Runs the add-time safety checks on a candidate machine file and reports the
     *          outcome. A cycle, a self-import, too deep a chain and an unsaved host are hard
     *          refusals; an unreadable file is reported and still accepted, because a broken
     *          import must never cost the user their own document.
     * \return  False when the candidate must not be registered.
     **/
    static bool acceptMachine(SMIncludeModel& model, const QString& absoluteFilePath, QWidget* parent);

    //!< The browse filter for an imported state machine.
    static QStringList getMachineExtensions(void);

//////////////////////////////////////////////////////////////////////////
// Overrides
//////////////////////////////////////////////////////////////////////////
public:
    /**
     * \brief   Hands over the description text of whichever of the two forms the selected row
     *          uses.
     **/
    virtual void commitPendingEdits(void) override;

protected:
    /**
     * \brief   Brings the imported machine editor forward for a `.fsml` row, and leaves every
     *          other row to the shared file editor.
     **/
    virtual void selectedInclude(const IncludeEntry* entry) override;

    /**
     * \brief   Lists the states that host the machine and lets the author decide.
     **/
    virtual bool confirmRemove(uint32_t id) override;

    /**
     * \brief   Runs the machine checks on a location the user typed, but only when it names a
     *          file that exists: a path that resolves to nothing cannot close a cycle or be too
     *          deep, and the user may be about to create it. Validation reports it either way.
     **/
    virtual bool acceptLocation(const QString& location) override;

    /**
     * \brief   A picked `.fsml` is a registration too, so it passes the same add-time checks and
     *          is stored relative to the host document rather than to the workspace.
     **/
    virtual void commitBrowsedLocation(uint32_t id, const QString& absolutePath, const QString& relativePath) override;

    virtual bool eventFilter(QObject* watched, QEvent* event) override;

//////////////////////////////////////////////////////////////////////////
// Slots
//////////////////////////////////////////////////////////////////////////
private slots:
    void onAliasCommitted(void);
    void onMachineLocationCommitted(void);
    void onMachineBrowseClicked(void);
    void onUpdateVersionClicked(void);

//////////////////////////////////////////////////////////////////////////
// Hidden methods
//////////////////////////////////////////////////////////////////////////
private:
    void buildMachineDetails(void);

    //!< The resolution and version-drift sentence shown for one imported machine.
    QString machineStatusText(const IncludeEntry& entry) const;

//////////////////////////////////////////////////////////////////////////
// Member variables
//////////////////////////////////////////////////////////////////////////
private:
    SMIncludeModel&     mModel;             //!< The includes of the state machine being edited.
    QWidget*            mMachinePage;       //!< Imported machine editor.
    QLineEdit*          mMachineAlias;
    QLineEdit*          mMachineLocation;
    QPushButton*        mMachineBrowse;
    QPlainTextEdit*     mMachineDescription;
    QLabel*             mMachineStatus;     //!< Resolution and version-drift text for the selection.
    QPushButton*        mMachineUpdate;
};

#endif  // LUSAN_VIEW_SM_SMINCLUDE_HPP
