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

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qlabel.h>
#include <qlayout.h>
//Added by qt3to4:
#include <QPixmap>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QEvent>

#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kparts/part.h>
#include <kstandarddirs.h>
#include <kurllabel.h>
#include <qtooltip.h>
#include <libkcal/calendar.h>
#include <libkcal/resourcecalendar.h>
#include <libkcal/resourcelocal.h>
#include <libkcal/todo.h>
#include <libkdepim/kpimprefs.h>

#include "korganizeriface_stub.h"

#include "core.h"
#include "plugin.h"
#include "todoplugin.h"

#include "korganizer/stdcalendar.h"

#include "todosummarywidget.h"

TodoSummaryWidget::TodoSummaryWidget( TodoPlugin *plugin,
                                      QWidget *parent, const char *name )
  : Kontact::Summary( parent, name ), mPlugin( plugin )
{
  QVBoxLayout *mainLayout = new QVBoxLayout( this, 3, 3 );

  QPixmap icon = KGlobal::iconLoader()->loadIcon( "kontact_todo",
                   KIcon::Desktop, KIcon::SizeMedium );
  QWidget *header = createHeader( this, icon, i18n( "Pending To-dos" ) );
  mainLayout->addWidget( header );

  mLayout = new QGridLayout( mainLayout, 7, 6, 3 );
  mLayout->setRowStretch( 6, 1 );

  mCalendar = KOrg::StdCalendar::self();
  mCalendar->load();

  connect( mCalendar, SIGNAL( calendarChanged() ), SLOT( updateView() ) );
  connect( mPlugin->core(), SIGNAL( dayChanged( const QDate& ) ),
           SLOT( updateView() ) );

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
  config.setGroup( "Days" );
  int mDaysToGo = config.readNumEntry( "DaysToShow", 7 );

  config.setGroup( "Hide" );
  mHideInProgress = config.readBoolEntry( "InProgress", false );
  mHideOverdue = config.readBoolEntry( "Overdue", false );
  mHideCompleted = config.readBoolEntry( "Completed", true );
  mHideOpenEnded = config.readBoolEntry( "OpenEnded", true );
  mHideNotStarted = config.readBoolEntry( "NotStarted", false );

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

  KCal::Todo::List todos = mCalendar->todos();
  KCal::Todo::List prList;
  KCal::Todo::List::ConstIterator it;
  KCal::Todo *todo;

  if ( todos.count() > 0 ) {
    for ( it = todos.begin(); it != todos.end(); ++it ) {
      todo = *it;

      if ( todo->hasDueDate() ) {
        int daysTo = QDate::currentDate().daysTo( todo->dtDue().date() );
        if ( daysTo >= mDaysToGo )
          continue;
      }

      if ( mHideOverdue && overdue( todo ) )
        continue;
      if ( mHideInProgress && inProgress( todo ) )
        continue;
      if ( mHideCompleted && completed( todo ) )
        continue;
      if ( mHideOpenEnded && openEnded( todo ) )
        continue;
      if ( mHideNotStarted && notStarted( todo ) )
        continue;

      prList.append( todo );
    }

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

  // No reason to show the date year
  QString savefmt = KGlobal::locale()->dateFormat();
  KGlobal::locale()->setDateFormat( KGlobal::locale()->
                                    dateFormat().replace( 'Y', ' ' ) );

  int counter = 0;
  QLabel *label = 0;

  if ( prList.count() > 0 ) {

    KIconLoader loader( "korganizer" );
    QPixmap pm = loader.loadIcon( "todo", KIcon::Small );

    QString str;

    for ( it = prList.begin(); it != prList.end(); ++it ) {
      todo = *it;
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
      if ( todo->hasDueDate() ) {
        daysTo = QDate::currentDate().daysTo( todo->dtDue().date() );

        if ( daysTo == 0 ) {
          makeBold = true;
          str = i18n( "Today" );
        } else if ( daysTo == 1 ) {
          str = i18n( "Tomorrow" );
        } else {
          str = KGlobal::locale()->formatDate( todo->dtDue().date() );
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
      if ( todo->hasDueDate() ) {
        if ( daysTo > 0 ) {
          str = i18n( "in 1 day", "in %n days", daysTo );
        } else if ( daysTo < 0 ) {
          str = i18n( "1 day ago", "%n days ago", -daysTo );
        } else{
          str = i18n( "due" );
        }
      }
      label = new QLabel( str, this );
      label->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
      mLayout->addWidget( label, counter, 2 );
      mLabels.append( label );

      // Priority label
      str = "[" + QString::number( todo->priority() ) + "]";
      label = new QLabel( str, this );
      label->setAlignment( Qt::AlignRight | Qt::AlignVCenter );
      mLayout->addWidget( label, counter, 3 );
      mLabels.append( label );

      // Summary label
      str = todo->summary();
      if ( todo->relatedTo() ) { // show parent only, not entire ancestry
        str = todo->relatedTo()->summary() + ":" + str;
      }
      KURLLabel *urlLabel = new KURLLabel( todo->uid(), str, this );
      urlLabel->installEventFilter( this );
      urlLabel->setTextFormat( Qt::RichText );
      mLayout->addWidget( urlLabel, counter, 4 );
      mLabels.append( urlLabel );

      if ( !todo->description().isEmpty() ) {
        urlLabel->setToolTip( todo->description() );
      }

      // State text label
      str = stateStr( todo );
      label = new QLabel( str, this );
      label->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
      mLayout->addWidget( label, counter, 5 );
      mLabels.append( label );

      connect( urlLabel, SIGNAL( leftClickedURL( const QString& ) ),
               this, SLOT( selectEvent( const QString& ) ) );

      counter++;
    }
  }

  if ( counter == 0 ) {
    QLabel *noTodos = new QLabel(
      i18n( "No pending to-dos due within the next day",
            "No pending to-dos due within the next %n days",
            mDaysToGo ), this );
    noTodos->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
    mLayout->addWidget( noTodos, 0, 2 );
    mLabels.append( noTodos );
  }

  Q_FOREACH( label, mLabels )
    label->show();

  KGlobal::locale()->setDateFormat( savefmt );
}

void TodoSummaryWidget::selectEvent( const QString &uid )
{
  mPlugin->core()->selectPlugin( "kontact_todoplugin" );//ensure loaded
  KOrganizerIface_stub iface( "korganizer", "KOrganizerIface" );
  iface.editIncidence( uid );
}

bool TodoSummaryWidget::eventFilter( QObject *obj, QEvent* e )
{
  if ( obj->inherits( "KURLLabel" ) ) {
    KURLLabel* label = static_cast<KURLLabel*>( obj );
    if ( e->type() == QEvent::Enter )
      emit message( i18n( "Edit To-do: \"%1\"" ).arg( label->text() ) );
    if ( e->type() == QEvent::Leave )
      emit message( QString::null );
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
       todo->dtDue().date() < QDate::currentDate() )
    return true;

  return false;
}

bool TodoSummaryWidget::starts( KCal::Todo *todo )
{
  if ( todo->hasStartDate() &&
       todo->dtStart().date() == QDate::currentDate() )
    return true;

  return false;
}

bool TodoSummaryWidget::completed( KCal::Todo *todo )
{
  return todo->isCompleted();
}

bool TodoSummaryWidget::openEnded( KCal::Todo *todo )
{
  if ( !todo->hasDueDate() && !todo->isCompleted() )
    return true;
  else
    return false;
}

bool TodoSummaryWidget::inProgress( KCal::Todo *todo )
{
  if ( todo->percentComplete() > 0 )
    return true;

  if ( todo->hasStartDate() && todo->hasDueDate() &&
       todo->dtStart().date() < QDate::currentDate() &&
       QDate::currentDate() < todo->dtDue().date() )
    return true;

  return false;
}

bool TodoSummaryWidget::notStarted( KCal::Todo *todo )
{
  if ( todo->percentComplete() > 0 )
    return false;

  if ( !todo->hasStartDate() )
    return false;

  if ( todo->dtStart().date() >= QDate::currentDate() )
    return false;

  return true;
}

const QString TodoSummaryWidget::stateStr( KCal::Todo *todo )
{
  QString str1, str2;

  if ( openEnded( todo ) ) {
    str1 = i18n( "open-ended" );
  } else if ( overdue( todo ) ) {
    str1 = i18n( "overdue" );
  } else if ( starts( todo ) ) {
    str1 = i18n( "starts today" );
  }

  if ( notStarted( todo ) ) {
    str2 += i18n( "not-started" );
  } else if ( completed( todo ) ) {
    str2 += i18n( "completed" );
  } else if ( inProgress( todo ) ) {
    str2 += i18n( "in-progress " );
    str2 += " (" + QString::number( todo->percentComplete() ) + "%)";
  }

  if ( !str1.isEmpty() && !str2.isEmpty() )
    str1 += i18n( "," );

  return str1 + str2;
}

#include "todosummarywidget.moc"
