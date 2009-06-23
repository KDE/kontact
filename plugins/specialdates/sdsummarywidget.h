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
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#ifndef SDSUMMARYWIDGET_H
#define SDSUMMARYWIDGET_H

#include <kontactinterfaces/summary.h>
#include <kholidays/holidays.h>

namespace Kontact {
  class Plugin;
}
namespace KCal {
  class Event;
  class Calendar;
}
class QEvent;
class QGridLayout;
class QLabel;
class QWidget;

class SDSummaryWidget : public Kontact::Summary
{
  Q_OBJECT

  public:
    SDSummaryWidget( Kontact::Plugin *plugin, QWidget *parent );

    QStringList configModules() const;
    void configUpdated();
    void updateSummary( bool force = false )
    {
      Q_UNUSED( force );
      updateView();
    }

  protected:
    virtual bool eventFilter( QObject *obj, QEvent *e );

  private slots:
    void updateView();
    void popupMenu( const QString &uid );
    void mailContact( const QString &uid );
    void viewContact( const QString &uid );

  private:
    int span( KCal::Event *event );
    int dayof( KCal::Event *event, const QDate &date );
    bool initHolidays();
    void dateDiff( const QDate &date, int &days, int &years );
#if 0 //sebsauer
    KCal::ResourceCalendar *usingBirthdayResource();
    bool check( KCal::ResourceCalendar *cal, const QDate &date, const QString &summary );
#endif
    QGridLayout *mLayout;
    QList<QLabel*> mLabels;
    Kontact::Plugin *mPlugin;
    KCal::Calendar *mCalendar;
    int mDaysAhead;
    bool mShowBirthdaysFromKAB;
    bool mShowBirthdaysFromCal;
    bool mShowAnniversariesFromKAB;
    bool mShowAnniversariesFromCal;
    bool mShowHolidays;
    bool mShowSpecialsFromCal;

    KHolidays::HolidayRegion *mHolidays;
};

#endif
