/*
    This file is part of Kontact.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2004 Allen Winter <winter@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef SDSUMMARYWIDGET_H
#define SDSUMMARYWIDGET_H

#include <qptrlist.h>
#include <qwidget.h>

#include <libkcal/calendarresources.h>
#include <libkholidays/kholidays.h>

#include "summary.h"

namespace Kontact {
  class Plugin;
}

class QGridLayout;
class QLabel;

class SDSummaryWidget : public Kontact::Summary
{
  Q_OBJECT

  public:
    SDSummaryWidget( Kontact::Plugin *plugin, QWidget *parent,
                     const char *name = 0 );

    QStringList configModules() const;
    void configUpdated();

  protected:
    virtual bool eventFilter(QObject *obj, QEvent* e);

  private slots:
    void updateView();
    void popupMenu( const QString &uid );
    void mailContact( const QString &uid );
    void viewContact( const QString &uid );

  private:
    void dateDiff( const QDate &date, int &days, int &years );
    QGridLayout *mLayout;
    QPtrList<QLabel> mLabels;
    Kontact::Plugin *mPlugin;
    KCal::CalendarResources *mCalendar;
    int mDaysAhead;
    bool mShowBirthdaysFromKAB;
    bool mShowBirthdaysFromCal;
    bool mShowAnniversariesFromKAB;
    bool mShowAnniversariesFromCal;
    bool mShowHolidays;
    bool mShowHolidaysFromCal;
    bool mShowSpecialsFromCal;
    KHolidays::KHolidays *mHolidays;
};

#endif
