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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qlabel.h>
#include <qlayout.h>

#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kparts/part.h>
#include <kstandarddirs.h>
#include <kurllabel.h>
#include <qtooltip.h>
#include <libkcal/resourcecalendar.h>
#include <libkcal/resourcelocal.h>
#include <libkcal/todo.h>
#include <libkdepim/kpimprefs.h>

#include <korganizer/stdcalendar.h>

#include "core.h"
#include "plugin.h"
#include "todoplugin.h"

#include "todosummarywidget.h"

TodoSummaryWidget::TodoSummaryWidget( TodoPlugin *plugin,
                                      QWidget *parent, const char *name )
  : Kontact::Summary( parent, name ), mPlugin( plugin )
{
  QVBoxLayout *mainLayout = new QVBoxLayout( this, 3, 3 );

  QPixmap icon = KGlobal::iconLoader()->loadIcon( "kontact_todo",
                   KIcon::Desktop, KIcon::SizeMedium );
  QWidget *header = createHeader( this, icon, i18n( "To-dos" ) );
  mainLayout->addWidget( header );

  mLayout = new QGridLayout( mainLayout, 7, 5, 3 );
  mLayout->setRowStretch( 6, 1 );

  mCalendar = KOrg::StdCalendar::self();
  mCalendar->load();

  connect( mCalendar, SIGNAL( calendarChanged() ), SLOT( updateView() ) );
  connect( mPlugin->core(), SIGNAL( dayChanged( const QDate& ) ),
           SLOT( updateView() ) );

  updateView();
}

void TodoSummaryWidget::updateView()
{
  mLabels.setAutoDelete( true );
  mLabels.clear();
  mLabels.setAutoDelete( false );

  KConfig config( "kcmkorgsummaryrc" );
  config.setGroup( "Todo" );
  bool showAllTodos = config.readBoolEntry( "ShowAllTodos", false );

  KIconLoader loader( "korganizer" );

  QLabel *label = 0;
  int counter = 0;

  KCal::Todo::List todos = mCalendar->todos();
  if ( todos.count() > 0 ) {
    QPixmap pm = loader.loadIcon( "todo", KIcon::Small );
    KCal::Todo::List::ConstIterator it;
    for ( it = todos.begin(); it != todos.end(); ++it ) {
      KCal::Todo *todo = *it;

      bool accepted = false;
      QString stateText;

      // show all incomplete todos
      if ( showAllTodos && !todo->isCompleted())
        accepted = accepted || true;

      // show uncomplete todos from the last days
      if ( todo->hasDueDate() && !todo->isCompleted() &&
           todo->dtDue().date() < QDate::currentDate() ) {
        accepted = accepted || true;
        stateText = i18n( "overdue" );
      }

      // show todos which started somewhere in the past and has to be finished in future
      if ( todo->hasStartDate() && todo->hasDueDate() && todo->dtStart().date()
           < QDate::currentDate() && QDate::currentDate() < todo->dtDue().date() ) {
        accepted = accepted || true;
        stateText = i18n( "in progress" );
      }

      // all todos which start today
      if ( todo->hasStartDate() && todo->dtStart().date() == QDate::currentDate() ) {
        accepted = accepted || true;
        stateText = i18n( "starts today" );
      }

      // all todos which end today
      if ( todo->hasDueDate() && todo->dtDue().date() == QDate::currentDate() ) {
        accepted = accepted || true;
        stateText = i18n( "ends today" );
      }

      if ( !accepted )
        continue;

      label = new QLabel( this );
      label->setPixmap( pm );
      label->setMaximumSize( label->minimumSizeHint() );
      mLayout->addWidget( label, counter, 0 );
      mLabels.append( label );

      label = new QLabel( QString::number( todo->percentComplete() ) + "%", this );
      label->setAlignment( AlignHCenter | AlignVCenter );
      mLayout->addWidget( label, counter, 1 );
      mLabels.append( label );

      QString sSummary = todo->summary();
      if ( todo->relatedTo() ) { // show parent only, not entire ancestry
        sSummary = todo->relatedTo()->summary() + ":" + todo->summary();
      }
      KURLLabel *urlLabel = new KURLLabel( todo->uid(), sSummary, this );
      urlLabel->setTextFormat( Qt::RichText );
      mLayout->addWidget( urlLabel, counter, 2 );
      mLabels.append( urlLabel );

      if ( !todo->description().isEmpty() ) {
        QToolTip::add( urlLabel, todo->description() );
      }

      label = new QLabel( stateText, this );
      label->setAlignment( AlignLeft | AlignVCenter );
      mLayout->addWidget( label, counter, 3 );
      mLabels.append( label );

      connect( urlLabel, SIGNAL( leftClickedURL( const QString& ) ),
               this, SLOT( selectEvent( const QString& ) ) );

      counter++;
    }
  }

  if ( counter == 0 ) {
    QLabel *noTodos = new QLabel( i18n( "No to-dos pending" ), this );
    noTodos->setAlignment( AlignRight );
    mLayout->addWidget( noTodos, 0, 2 );
    mLabels.append( noTodos );
  }

  for ( label = mLabels.first(); label; label = mLabels.next() )
    label->show();
}

void TodoSummaryWidget::selectEvent( const QString & )
{
  mPlugin->core()->selectPlugin( "kontact_todoplugin" );
  mPlugin->interface()->showTodoView();
}

QStringList TodoSummaryWidget::configModules() const
{
  return QStringList( "kcmkorgsummary.desktop" );
}

#include "todosummarywidget.moc"
