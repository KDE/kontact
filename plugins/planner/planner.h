/*
  This file is part of Kontact.

  Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

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

#ifndef PLANNERWIDGET_H
#define PLANNERWIDGET_H

#include <KontactInterface/Summary>

#include <KCal/Event>
#include <KCal/Todo>

#include <QList>

class SDEntry;

namespace KABC {
  class AddressBook;
}
using namespace KABC;

namespace KCal {
  class Calendar;
}
using namespace KCal;

namespace KHolidays {
  class HolidayRegion;
}

namespace KontactInterface {
  class Plugin;
}

class QGridLayout;
class QLabel;
class QVBoxLayout;

class Planner : public KontactInterface::Summary
{
  Q_OBJECT

  public:
    Planner( KontactInterface::Plugin *plugin, QWidget *parent );

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
    virtual bool todoEventFilter( QObject *obj, QEvent *e );

  private slots:
    void initTodoList( const QDate &date );
    int showTodos( int counter, const QDate &date );
    void initEventList( const QDate &date );
    int showEvents( int counter, const QDate &date );
    void initSdList( const QDate &date );
    int showSd( int counter, const QDate &date );
    void updateView();

    void eventPopupMenu( const QString &uid );
    void viewEvent( const QString &uid );
    void removeEvent( const QString &uid );
    void todoPopupMenu( const QString &uid );
    void viewTodo( const QString &uid );
    void removeTodo( const QString &uid );
    void completeTodo( const QString &uid );
    void changePercentage( const QString &uid );

    bool inProgress( Todo *todo );
    bool overdue( Todo *todo );
    bool completed( Todo *todo );
    bool openEnded( Todo *todo );
    bool notStarted( Todo *todo );

  private:
    bool mShowRecurrence;
    bool mShowReminder;
    bool mUnderline;
    bool mTodo;
    bool mSd;

    int mCustomDays;

    bool mHideCompleted;
    bool mHideOpenEnded;
    bool mHideNotStarted;
    bool mHideInProgress;
    bool mHideOverdue;

    bool initHolidays();
    bool mBirthdayCal;
    bool mBirthdayConList;
    bool mAnniversariesCal;
    bool mAnniversariesConList;
    bool mHolidaysCal;
    bool mSpecialOccasionsCal;

    bool mShowMyEventsOnly;
    bool mShowMyTodosOnly;

    Event::List mEvents;
    KontactInterface::Plugin *mPlugin;
    QVBoxLayout *mLayout;
    QGridLayout *mPlannerGrid;
    QList<QLabel *> mLabels;

    KCal::Calendar *mCalendar;
    AddressBook *mAddressBook;

    Todo::List mTodos;
    QString initStateText( const Todo *todo, const QDate &date );
    KHolidays::HolidayRegion *mHolidays;
    QList<SDEntry> mDates;
};

#endif
