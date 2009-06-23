/*
  This file is part of Kontact.

  Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>
  Copyright (c) 2005-2006,2008-2009 Allen Winter <winter@kde.org>

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

#include "todosummarywidget.h"
#include "todoplugin.h"
#include "korganizerinterface.h"

#include <korganizer/stdcalendar.h>
#include <korganizer/koglobals.h>
#include <kontactinterfaces/core.h>

#include <libkdepim/kpimprefs.h>

#include <kcal/calendar.h>
#include <kcal/resourcelocal.h>
#include <kcal/todo.h>
#include <kcal/incidenceformatter.h>

#include <kdialog.h>
#include <kglobal.h>
#include <kicon.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kmenu.h>
#include <kstandarddirs.h>
#include <kurllabel.h>
#include <kparts/part.h>

#include <QCursor>
#include <QEvent>
#include <QGridLayout>
#include <QLabel>
#include <QLayout>
#include <QPixmap>
#include <QVBoxLayout>
#include <QTextDocument>

TodoSummaryWidget::TodoSummaryWidget( TodoPlugin *plugin, QWidget *parent )
  : Kontact::Summary( parent ), mPlugin( plugin )
{
  QVBoxLayout *mainLayout = new QVBoxLayout( this );
  mainLayout->setSpacing( 3 );
  mainLayout->setMargin( 3 );

  QWidget *header = createHeader( this, "view-pim-tasks", i18n( "Pending To-dos" ) );
  mainLayout->addWidget( header );

  mLayout = new QGridLayout();
  mainLayout->addItem( mLayout );
  mLayout->setSpacing( 3 );
  mLayout->setRowStretch( 6, 1 );

  mCalendar = KOrg::StdCalendar::self();
  
  //If the kpart isn't created yet, it's created now, and mCalendar is loaded
  mPlugin->part();

  connect( mCalendar, SIGNAL(calendarChanged()), SLOT(updateView()) );
  connect( mPlugin->core(), SIGNAL(dayChanged(const QDate&)), SLOT(updateView()) );

  updateView();
}

TodoSummaryWidget::~TodoSummaryWidget()
{
}

void TodoSummaryWidget::updateView()
{
  qDeleteAll( mLabels );
  mLabels.clear();

  KConfig config( "kcmtodosummaryrc" );
  KConfigGroup daysGroup( &config, "Days" );
  int mDaysToGo = daysGroup.readEntry( "DaysToShow", 7 );

  KConfigGroup hideGroup( &config, "Hide" );
  mHideInProgress = hideGroup.readEntry( "InProgress", false );
  mHideOverdue = hideGroup.readEntry( "Overdue", false );
  mHideCompleted = hideGroup.readEntry( "Completed", true );
  mHideOpenEnded = hideGroup.readEntry( "OpenEnded", true );
  mHideNotStarted = hideGroup.readEntry( "NotStarted", false );

  // for each todo,
  //   if it passes the filter, append to a list
  //   else continue
  // sort todolist by summary
  // sort todolist by priority
  // sort todolist by due-date
  // print todolist

  // the filter is created by the configuration summary options, but includes
  //    days to go before to-do is due
  //    which types of to-dos to hide

  KCal::Todo::List prList;

  QDate currDate = QDate::currentDate();
  Q_FOREACH ( KCal::Todo *todo, mCalendar->todos() ) {
    if ( todo->hasDueDate() ) {
      int daysTo = currDate.daysTo( todo->dtDue().date() );
      if ( daysTo >= mDaysToGo ) {
        continue;
      }
    }

    if ( mHideOverdue && overdue( todo ) ) {
      continue;
    }
    if ( mHideInProgress && inProgress( todo ) ) {
      continue;
    }
    if ( mHideCompleted && completed( todo ) ) {
      continue;
    }
    if ( mHideOpenEnded && openEnded( todo ) ) {
      continue;
    }
    if ( mHideNotStarted && notStarted( todo ) ) {
      continue;
    }

    prList.append( todo );
  }
  if ( !prList.isEmpty() ) {
    prList = KCal::Calendar::sortTodos( &prList,
                                        KCal::TodoSortSummary,
                                        KCal::SortDirectionAscending );
    prList = KCal::Calendar::sortTodos( &prList,
                                        KCal::TodoSortPriority,
                                        KCal::SortDirectionAscending );
    prList = KCal::Calendar::sortTodos( &prList,
                                        KCal::TodoSortDueDate,
                                        KCal::SortDirectionAscending );
  }

  // The to-do print consists of the following fields:
  //  icon:due date:days-to-go:priority:summary:status
  // where,
  //   the icon is the typical to-do icon
  //   the due date it the to-do due date
  //   the days-to-go/past is the #days until/since the to-do is due
  //     this field is left blank if the to-do is open-ended
  //   the priority is the to-do priority
  //   the summary is the to-do summary
  //   the status is comma-separated list of:
  //     overdue
  //     in-progress (started, or >0% completed)
  //     complete (100% completed)
  //     open-ended
  //     not-started (no start date and 0% completed)

  int counter = 0;
  QLabel *label = 0;

  if ( !prList.isEmpty() ) {

    KIconLoader loader( "korganizer" );
    QPixmap pm = loader.loadIcon( "view-calendar-tasks", KIconLoader::Small );

    QString str;

    Q_FOREACH ( KCal::Todo *todo, prList ) {
      bool makeBold = false;
      int daysTo = -1;

      // Icon label
      label = new QLabel( this );
      label->setPixmap( pm );
      label->setMaximumWidth( label->minimumSizeHint().width() );
      mLayout->addWidget( label, counter, 0 );
      mLabels.append( label );

      // Due date label
      str = "";
      if ( todo->hasDueDate() && todo->dtDue().date().isValid() ) {
        daysTo = currDate.daysTo( todo->dtDue().date() );

        if ( daysTo == 0 ) {
          makeBold = true;
          str = i18nc( "the to-do is due today", "Today" );
        } else if ( daysTo == 1 ) {
          str = i18nc( "the to-do is due tomorrow", "Tomorrow" );
        } else {
          str = KGlobal::locale()->formatDate( todo->dtDue().date(), KLocale::FancyLongDate );
        }
      }

      label = new QLabel( str, this );
      label->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
      mLayout->addWidget( label, counter, 1 );
      mLabels.append( label );
      if ( makeBold ) {
        QFont font = label->font();
        font.setBold( true );
        label->setFont( font );
      }

      // Days togo/ago label
      str = "";
      if ( todo->hasDueDate() && todo->dtDue().date().isValid() ) {
        if ( daysTo > 0 ) {
          str = i18np( "in 1 day", "in %1 days", daysTo );
        } else if ( daysTo < 0 ) {
          str = i18np( "1 day ago", "%1 days ago", -daysTo );
        } else {
          str = i18nc( "the to-do is due", "due" );
        }
      }
      label = new QLabel( str, this );
      label->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
      mLayout->addWidget( label, counter, 2 );
      mLabels.append( label );

      // Priority label
      str = '[' + QString::number( todo->priority() ) + ']';
      label = new QLabel( str, this );
      label->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
      mLayout->addWidget( label, counter, 3 );
      mLabels.append( label );

      // Summary label
      str = todo->summary();
      if ( todo->relatedTo() ) { // show parent only, not entire ancestry
        str = todo->relatedTo()->summary() + ':' + str;
      }
      if ( !Qt::mightBeRichText( str ) ) {
        str = Qt::escape( str );
      }

      KUrlLabel *urlLabel = new KUrlLabel( this );
      urlLabel->setText( str );
      urlLabel->setUrl( todo->uid() );
      urlLabel->installEventFilter( this );
      urlLabel->setTextFormat( Qt::RichText );
      urlLabel->setWordWrap( true );
      mLayout->addWidget( urlLabel, counter, 4 );
      mLabels.append( urlLabel );

      connect( urlLabel, SIGNAL(leftClickedUrl(const QString&)),
               this, SLOT(viewTodo(const QString&)) );
      connect( urlLabel, SIGNAL(rightClickedUrl(const QString&)),
               this, SLOT(popupMenu(const QString&)) );

      QString tipText( KCal::IncidenceFormatter::toolTipStr(
                         todo, true, KPIM::KPimPrefs::timeSpec() ) );
      if ( !tipText.isEmpty() ) {
        urlLabel->setToolTip( tipText );
      }

      // State text label
      str = stateStr( todo );
      label = new QLabel( str, this );
      label->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
      mLayout->addWidget( label, counter, 5 );
      mLabels.append( label );

      counter++;
    }
  } //foreach

  if ( counter == 0 ) {
    QLabel *noTodos = new QLabel(
      i18np( "No pending to-dos due within the next day",
             "No pending to-dos due within the next %1 days",
             mDaysToGo ), this );
    noTodos->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
    mLayout->addWidget( noTodos, 0, 0 );
    mLabels.append( noTodos );
  }

  Q_FOREACH( label, mLabels ) { //krazy:exclude=foreach as label is a pointer
    label->show();
  }
}

void TodoSummaryWidget::viewTodo( const QString &uid )
{
  mPlugin->core()->selectPlugin( "kontact_todoplugin" );//ensure loaded
  OrgKdeKorganizerKorganizerInterface korganizer(
    "org.kde.korganizer", "/Korganizer", QDBusConnection::sessionBus() );
  korganizer.editIncidence( uid );
}

void TodoSummaryWidget::removeTodo( const QString &uid )
{
  mPlugin->core()->selectPlugin( "kontact_todoplugin" );//ensure loaded
  OrgKdeKorganizerKorganizerInterface korganizer(
    "org.kde.korganizer", "/Korganizer", QDBusConnection::sessionBus() );
  korganizer.deleteIncidence( uid, false );
}

void TodoSummaryWidget::completeTodo( const QString &uid )
{
  KCal::Todo *todo = mCalendar->todo( uid );
  if ( !todo->isReadOnly() ) {
    todo->setCompleted( KDateTime::currentLocalDateTime() );
    mCalendar->save();
    updateView();
  }
}

void TodoSummaryWidget::popupMenu( const QString &uid )
{
  KMenu popup( this );
  QAction *editIt = popup.addAction( i18n( "&Edit To-do..." ) );
  QAction *delIt = popup.addAction( i18n( "&Delete To-do" ) );
  delIt->setIcon( KIconLoader::global()->loadIcon( "edit-delete", KIconLoader::Small ) );
  QAction *doneIt = 0;
  KCal::Todo *todo = mCalendar->todo( uid );
  if ( !todo->isCompleted() ) {
    doneIt = popup.addAction( i18n( "&Mark To-do Completed" ) );
    doneIt->setIcon( KIconLoader::global()->loadIcon( "task-complete", KIconLoader::Small ) );
  }
  // TODO: add icons to the menu actions

  const QAction *selectedAction = popup.exec( QCursor::pos() );
  if ( selectedAction == editIt ) {
    viewTodo( uid );
  } else if ( selectedAction == delIt ) {
    removeTodo( uid );
  } else if ( doneIt && selectedAction == doneIt ) {
    completeTodo( uid );
  }
}

bool TodoSummaryWidget::eventFilter( QObject *obj, QEvent *e )
{
  if ( obj->inherits( "KUrlLabel" ) ) {
    KUrlLabel* label = static_cast<KUrlLabel*>( obj );
    if ( e->type() == QEvent::Enter ) {
      emit message( i18n( "Edit To-do: \"%1\"", label->text() ) );
    }
    if ( e->type() == QEvent::Leave ) {
      emit message( QString::null );	//krazy:exclude=nullstrassign for old broken gcc
    }
  }
  return Kontact::Summary::eventFilter( obj, e );
}

QStringList TodoSummaryWidget::configModules() const
{
  return QStringList( "kcmtodosummary.desktop" );
}

bool TodoSummaryWidget::overdue( KCal::Todo *todo )
{
  if ( todo->hasDueDate() && !todo->isCompleted() &&
       todo->dtDue().date() < QDate::currentDate() ) {
    return true;
  }
  return false;
}

bool TodoSummaryWidget::starts( KCal::Todo *todo )
{
  if ( todo->hasStartDate() &&
       todo->dtStart().date() == QDate::currentDate() ) {
    return true;
  }
  return false;
}

bool TodoSummaryWidget::completed( KCal::Todo *todo )
{
  return todo->isCompleted();
}

bool TodoSummaryWidget::openEnded( KCal::Todo *todo )
{
  if ( !todo->hasDueDate() && !todo->isCompleted() ) {
    return true;
  }
  return false;
}

bool TodoSummaryWidget::inProgress( KCal::Todo *todo )
{
  if ( todo->percentComplete() > 0 ) {
    return true;
  }

  QDate currDate = QDate::currentDate();
  if ( todo->hasStartDate() && todo->hasDueDate() &&
       todo->dtStart().date() < currDate &&
       currDate < todo->dtDue().date() ) {
    return true;
  }

  return false;
}

bool TodoSummaryWidget::notStarted( KCal::Todo *todo )
{
  if ( todo->percentComplete() > 0 ) {
    return false;
  }

  if ( !todo->hasStartDate() ) {
    return false;
  }

  if ( todo->dtStart().date() >= QDate::currentDate() ) {
    return false;
  }

  return true;
}

const QString TodoSummaryWidget::stateStr( KCal::Todo *todo )
{
  QString str1, str2;

  if ( openEnded( todo ) ) {
    str1 = i18n( "open-ended" );
  } else if ( overdue( todo ) ) {
    str1 = "<font color=\"red\">" +
           i18nc( "the to-do is overdue", "overdue" ) +
           "</font>";
  } else if ( starts( todo ) ) {
    str1 = i18nc( "the to-do starts today", "starts today" );
  }

  if ( notStarted( todo ) ) {
    str2 += i18nc( "the to-do has not been started yet", "not-started" );
  } else if ( completed( todo ) ) {
    str2 += i18nc( "the to-do is completed", "completed" );
  } else if ( inProgress( todo ) ) {
    str2 += i18nc( "the to-do is in-progress", "in-progress " );
    str2 += " (" + QString::number( todo->percentComplete() ) + "%)";
  }

  if ( !str1.isEmpty() && !str2.isEmpty() ) {
    str1 += i18nc( "Separator for status like this: overdue, completed", "," );
  }

  return str1 + str2;
}

#include "todosummarywidget.moc"
