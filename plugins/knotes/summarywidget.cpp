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

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "summarywidget.h"

#include <kontactinterfaces/core.h>
#include <kontactinterfaces/plugin.h>

#include <knotes/resourcenotes.h>
#include <knotes/resourcemanager.h>

#include <kcal/calendarlocal.h>

#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kurllabel.h>
#include <kstandarddirs.h>

#include <QObject>
#include <QLabel>
#include <QLayout>
#include <QVBoxLayout>
#include <QPixmap>
#include <QGridLayout>
#include <QEvent>

KNotesSummaryWidget::KNotesSummaryWidget( Kontact::Plugin *plugin, QWidget *parent )
  : Kontact::Summary( parent ), mLayout( 0 ), mPlugin( plugin )
{
  QVBoxLayout *mainLayout = new QVBoxLayout( this );
  mainLayout->setSpacing( 3 );
  mainLayout->setMargin( 3 );

  QWidget *header = createHeader( this, "view-pim-notes", i18n( "Popup Notes" ) );
  mainLayout->addWidget( header );

  mLayout = new QGridLayout();
  mainLayout->addItem( mLayout );
  mLayout->setSpacing( 3 );
  mLayout->setRowStretch( 6, 1 );

  mCalendar = new KCal::CalendarLocal( QString::fromLatin1( "UTC" ) );
  KNotesResourceManager *manager = new KNotesResourceManager();

  QObject::connect( manager, SIGNAL(sigRegisteredNote(KCal::Journal*)),
                    this, SLOT(addNote(KCal::Journal*)) );
  QObject::connect( manager, SIGNAL(sigDeregisteredNote(KCal::Journal*)),
                    this, SLOT(removeNote(KCal::Journal*)) );
  manager->load();

  updateView();
}

void KNotesSummaryWidget::updateView()
{
  mNotes = mCalendar->journals();
  QLabel *label = 0;

  Q_FOREACH ( label, mLabels ) {
    label->deleteLater();
  }
  mLabels.clear();

  KIconLoader loader( "knotes" );

  int counter = 0;
  QPixmap pm = loader.loadIcon( "knotes", KIconLoader::Small );

  KCal::Journal::List::Iterator it;
  if ( mNotes.count() ) {
    for ( it = mNotes.begin(); it != mNotes.end(); ++it ) {

      // Fill Note Pixmap Field
      label = new QLabel( this );
      label->setPixmap( pm );
      label->setMaximumWidth( label->minimumSizeHint().width() );
      label->setAlignment( Qt::AlignVCenter );
      mLayout->addWidget( label, counter, 0 );
      mLabels.append( label );

      // File Note Summary Field
      QString newtext = (*it)->summary();

      KUrlLabel *urlLabel = new KUrlLabel( (*it)->uid(), newtext, this );
      urlLabel->installEventFilter( this );
      urlLabel->setTextFormat( Qt::RichText );
      urlLabel->setAlignment( Qt::AlignLeft );
      urlLabel->setWordWrap( true );
      mLayout->addWidget( urlLabel, counter, 1 );
      mLabels.append( urlLabel );

      if ( !(*it)->description().isEmpty() ) {
        urlLabel->setToolTip( (*it)->description().left( 80 ) );
      }

      connect( urlLabel, SIGNAL(leftClickedUrl(const QString&)),
               this, SLOT(urlClicked(const QString&)) );
      counter++;
    }

  } else {
      QLabel *noNotes = new QLabel( i18n( "No Notes Available" ), this );
      noNotes->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
      mLayout->addWidget( noNotes, 0, 0 );
      mLabels.append( noNotes );
  }

  Q_FOREACH( label, mLabels ) { //krazy:exclude=foreach as label is a pointer
    label->show();
  }
}

void KNotesSummaryWidget::urlClicked( const QString &/*uid*/ )
{
  if ( !mPlugin->isRunningStandalone() ) {
    mPlugin->core()->selectPlugin( mPlugin );
  } else {
    mPlugin->bringToForeground();
  }
}

bool KNotesSummaryWidget::eventFilter( QObject *obj, QEvent *e )
{
  if ( obj->inherits( "KUrlLabel" ) ) {
    KUrlLabel* label = static_cast<KUrlLabel*>( obj );
    if ( e->type() == QEvent::Enter ) {
      emit message( i18n( "Read Popup Note: \"%1\"", label->text() ) );
    }
    if ( e->type() == QEvent::Leave ) {
      emit message( QString::null );	//krazy:exclude=nullstrassign for old broken gcc
    }
  }

  return Kontact::Summary::eventFilter( obj, e );
}

void KNotesSummaryWidget::addNote( KCal::Journal *j )
{
  mCalendar->addJournal( j );
  updateView();
}

void KNotesSummaryWidget::removeNote( KCal::Journal *j )
{
  mCalendar->deleteJournal( j );
  updateView();
}

#include "summarywidget.moc"
