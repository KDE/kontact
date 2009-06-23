/*
  This file is part of Kontact.

  Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>
  Copyright (c) 2005-2006 Allen Winter <winter@kde.org>

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

#ifndef TODO_SUMMARYWIDGET_H
#define TODO_SUMMARYWIDGET_H

#include <kontactinterfaces/summary.h>
#include <QList>

namespace KCal {
  class Calendar;
  class Todo;
}

class TodoPlugin;

class QEvent;
class QGridLayout;
class QLabel;
class QWidget;

class TodoSummaryWidget : public Kontact::Summary
{
  Q_OBJECT

  public:
    TodoSummaryWidget( TodoPlugin *plugin, QWidget *parent );
    ~TodoSummaryWidget();

    int summaryHeight() const { return 3; }
    QStringList configModules() const;

  public slots:
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
    void viewTodo( const QString &uid );
    void removeTodo( const QString &uid );
    void completeTodo( const QString &uid );

  private:
    TodoPlugin *mPlugin;
    QGridLayout *mLayout;

    bool mDaysToGo;
    bool mHideInProgress;
    bool mHideOverdue;
    bool mHideCompleted;
    bool mHideOpenEnded;
    bool mHideNotStarted;

    QList<QLabel*> mLabels;
    KCal::Calendar *mCalendar;

    // FIXME: could the following methods be moved into the KCal::Todo class?

    /**
       Test if a To-do is in-progress.
       @param todo is a pointer to a To-do object to test.
       @return if the To-do is in-progress.
    */
    bool inProgress( KCal::Todo *todo );
    /**
       Test if a To-do is overdue.
       @param todo is a pointer to a To-do object to test.
       @return if the To-do is overdue.
    */
    bool overdue( KCal::Todo *todo );
    /**
       Test if a To-do is completed.
       @param todo is a pointer to a To-do object to test.
       @return if the To-do is overdue.
    */
    bool starts( KCal::Todo *todo );
    /**
       Test if a To-do starts on the current date.
       @param todo is a pointer to a To-do object to test.
       @return if the To-do starts today.
    */
    bool completed( KCal::Todo *todo );
    /**
       Test if a To-do is open-ended.
       @param todo is a pointer to a To-do object to test.
       @return if the To-do is completed.
    */
    bool openEnded( KCal::Todo *todo );
    /**
       Test if a To-do has yet to be started.
       @param todo is a pointer to a To-do object to test.
       @return if the To-do is yet to be started.
    */
    bool notStarted( KCal::Todo *todo );

    /**
       Create a text string containing the states of the To-do.
       @param todo is a pointer to a To-do object to test.
       @return a QString containing a comma-separated list of To-do states.
    */
    const QString stateStr( KCal::Todo *todo );
};

#endif
