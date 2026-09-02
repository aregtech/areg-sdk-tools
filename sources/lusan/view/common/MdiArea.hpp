#ifndef LUSAN_VIEW_COMMON_MDIAREA_HPP
#define LUSAN_VIEW_COMMON_MDIAREA_HPP
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
 *  \file        lusan/view/common/MdiArea.hpp
 *  \ingroup     Lusan - GUI Tool for Areg SDK
 *  \author      Artak Avetyan
 *  \brief       Lusan application, MDI Area.
 *
 ************************************************************************/

#include <QMdiArea>
#include <QWidget>

class QEvent;

/**
 * \brief   The editor area the document windows live in.
 *
 *          Takes its background from the active palette and re-takes it whenever the
 *          palette changes, so the area follows a theme switch.
 **/
class MdiArea : public QMdiArea
{
    Q_OBJECT
public:
    MdiArea(QWidget * parent = nullptr);

protected:
    /**
     * \brief   Re-takes the background brush when the palette changes.
     **/
    virtual void changeEvent(QEvent* event) override;

private:
    //!< Reads the background brush out of the palette in force.
    inline void applyThemeBackground(void);
};

#endif // LUSAN_VIEW_COMMON_MDIAREA_HPP
