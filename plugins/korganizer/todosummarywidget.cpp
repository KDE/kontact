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
#include "korganizer/korganizerinterface.h"

#include <KontactInterface/Core>

#include <KCal/CalHelper>
#include <KCal/IncidenceFormatter>
#include <KCal/Todo>

#include <akonadi/kcal/calendar.h>
#include <akonadi/kcal/calendaradaptor.h>
#include <akonadi/kcal/calendarmodel.h>
#include <Akonadi/ChangeRecorder>
#include <Akonadi/Session>
#include <Akonadi/Collection>
#include <Akonadi/ItemFetchScope>
#include <akonadi/entitydisplayattribute.h>
#include <akonadi/kcal/incidencemimetypevisitor.h>
#include <akonadi/kcal/utils.h>

#include <KConfigGroup>
#include <KIconLoader>
#include <KLocale>
#include <KMenu>
#include <KSystemTimeZones>
#include <KUrlLabel>

#include <QGridLayout>
#include <QLabel>
#include <QTextDocument>  // for Qt::mightBeRichText
#include <QVBoxLayout>

using namespace Akonadi;

TodoSummaryWidget::TodoSummaryWidget( TodoPlugin *plugin, QWidget *parent )
  : KontactInterface::Summary( parent ), mPlugin( plugin ), mCalendar( 0 )
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

  createCalendar();

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
  KConfigGroup group = config.group( "Days" );
  int mDaysToGo = group.readEntry( "DaysToShow", 7 );

  group = config.group( "Hide" );
  mHideInProgress = group.readEntry( "InProgress", false );
  mHideOverdue = group.readEntry( "Overdue", false );
  mHideCompleted = group.readEntry( "Completed", true );
  mHideOpenEnded = group.readEntry( "OpenEnded", true );
  mHideNotStarted = group.readEntry( "NotStarted", false );

  group = config.group( "Groupware" );
  mShowMineOnly = group.readEntry( "ShowMineOnly", false );

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

  Item::List prList;

  QDate currDate = QDate::currentDate();
  Q_FOREACH ( const Item &todoItem, mCalendar->todos() ) {
    KCal::Todo::Ptr todo = Akonadi::todo( todoItem );
    if ( todo->hasDueDate() ) {
      int daysTo = currDate.daysTo( todo->dtDue().date() );
      if ( daysTo >= mDaysToGo ) {
        continue;
      }
    }

    if ( mHideOverdue && todo->isOverdue() ) {
      continue;
    }
    if ( mHideInProgress && todo->isInProgress( false ) ) {
      continue;
    }
    if ( mHideCompleted && todo->isCompleted() ) {
      continue;
    }
    if ( mHideOpenEnded && todo->isOpenEnded() ) {
      continue;
    }
    if ( mHideNotStarted && todo->isNotStarted( false ) ) {
      continue;
    }

    prList.append( todoItem );
  }
  if ( !prList.isEmpty() ) {
    prList = Calendar::sortTodos( prList,
                                  TodoSortSummary,
                                  SortDirectionAscending );
    prList = Calendar::sortTodos( prList,
                                  TodoSortPriority,
                                  SortDirectionAscending );
    prList = Calendar::sortTodos( prList,
                                  TodoSortDueDate,
                                  SortDirectionAscending );
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

    Q_FOREACH ( const Item &todoItem, prList ) {
      KCal::Todo::Ptr todo = Akonadi::todo( todoItem );
      bool makeBold = false;
      int daysTo = -1;

      // Optionally, show only my To-dos
      if ( mShowMineOnly && !KCal::CalHelper::isMyCalendarIncidence( mCalendarAdaptor, todo.get() ) ) {
        continue;
      }

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
                         mCalendarAdaptor, todo.get(), currDate, true, KSystemTimeZones::local() ) );
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

void TodoSummaryWidget::completeTodo( const Item::Id &id )
{
  Item todoItem = mCalendar->todo( id );

  if ( todoItem.isValid() ) {
    KCal::Todo::Ptr todo = Akonadi::todo( todoItem );
  
    if ( !todo->isReadOnly() ) {
      todo->setCompleted( KDateTime::currentLocalDateTime() );
      // TODO, use incidenceChanger
      updateView();
    }
  }
}

void TodoSummaryWidget::popupMenu( const QString &uid )
{
  KMenu popup( this );
  QAction *editIt = popup.addAction( i18n( "&Edit To-do..." ) );
  QAction *delIt = popup.addAction( i18n( "&Delete To-do" ) );
  delIt->setIcon( KIconLoader::global()->loadIcon( "edit-delete", KIconLoader::Small ) );
  QAction *doneIt = 0;
  Item::Id id= mCalendar->itemIdForIncidenceUid( uid );
  Item todoItem = mCalendar->todo( id );
  KCal::Todo::Ptr todo = Akonadi::todo( todoItem );
  
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
    completeTodo( todoItem.id() );
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
  return KontactInterface::Summary::eventFilter( obj, e );
}

QStringList TodoSummaryWidget::configModules() const
{
  return QStringList( "kcmtodosummary.desktop" );
}

bool TodoSummaryWidget::startsToday( KCal::Todo::Ptr todo )
{
  return todo->hasStartDate() &&
         todo->dtStart().date() == QDate::currentDate();
}

const QString TodoSummaryWidget::stateStr( KCal::Todo::Ptr todo )
{
  QString str1, str2;

  if ( todo->isOpenEnded() ) {
    str1 = i18n( "open-ended" );
  } else if ( todo->isOverdue() ) {
    str1 = "<font color=\"red\">" +
           i18nc( "the to-do is overdue", "overdue" ) +
           "</font>";
  } else if ( startsToday( todo ) ) {
    str1 = i18nc( "the to-do starts today", "starts today" );
  }

  if ( todo->isNotStarted( false ) ) {
    str2 += i18nc( "the to-do has not been started yet", "not-started" );
  } else if ( todo->isCompleted() ) {
    str2 += i18nc( "the to-do is completed", "completed" );
  } else if ( todo->isInProgress( false ) ) {
    str2 += i18nc( "the to-do is in-progress", "in-progress " );
    str2 += " (" + QString::number( todo->percentComplete() ) + "%)";
  }

  if ( !str1.isEmpty() && !str2.isEmpty() ) {
    str1 += i18nc( "Separator for status like this: overdue, completed", "," );
  }

  return str1 + str2;
}

void TodoSummaryWidget::createCalendar()
{
  Session *session = new Session( "TodoSummaryWidget", this );
  ChangeRecorder *monitor = new ChangeRecorder( this );

  ItemFetchScope scope;
  scope.fetchFullPayload( true );
  scope.fetchAttribute<EntityDisplayAttribute>();

  monitor->setSession( session );
  monitor->setCollectionMonitored( Collection::root() );
  monitor->fetchCollection( true );
  monitor->setItemFetchScope( scope );
  monitor->setMimeTypeMonitored( Akonadi::IncidenceMimeTypeVisitor::todoMimeType(), true );
  CalendarModel *calendarModel = new CalendarModel( monitor, this );

  mCalendar = new Akonadi::Calendar( calendarModel, calendarModel, KSystemTimeZones::local() );
  mCalendarAdaptor = new CalendarAdaptor( mCalendar, this );
  
}

#include "todosummarywidget.moc"
