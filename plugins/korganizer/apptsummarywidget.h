/*
  This file is part of Kontact.

  Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>
  Copyright (c) 2005-2006,2008 Allen Winter <winter@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#ifndef SUMMARYWIDGET_H
#define SUMMARYWIDGET_H

#include <kontactinterfaces/summary.h>
#include <QList>

namespace KCal {
  class Calendar;
  class Event;
}

class KOrganizerPlugin;

class QDate;
class QEvent;
class QGridLayout;
class QLabel;
class QWidget;

class ApptSummaryWidget : public Kontact::Summary
{
  Q_OBJECT

  public:
    ApptSummaryWidget( KOrganizerPlugin *plugin, QWidget *parent );
    ~ApptSummaryWidget();

    int summaryHeight() const { return 3; }
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
    void viewEvent( const QString &uid );
    void removeEvent( const QString &uid );

  private:
    void dateDiff( const QDate &date, int &days );
    bool skip( KCal::Event *event );

    QGridLayout *mLayout;
    QList<QLabel *> mLabels;
    KOrganizerPlugin *mPlugin;
    KCal::Calendar *mCalendar;
    int mDaysAhead;
    bool mShowBirthdaysFromCal;
    bool mShowAnniversariesFromCal;
};

#endif
