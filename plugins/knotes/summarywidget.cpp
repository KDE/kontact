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

#include <dcopclient.h>
#include <dcopref.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kurllabel.h>
#include <kstandarddirs.h>

#include "core.h"
#include "plugin.h"

#include "summarywidget.h"

KNotesSummaryWidget::KNotesSummaryWidget( Kontact::Plugin *plugin,
        QWidget *parent, const char *name )
  : Kontact::Summary( parent, name ),
    mPlugin( plugin )
{
  mMainLayout = new QVBoxLayout( this, 3, 3 );

  mCalendar = new KCal::CalendarResources;
  mResource = new KCal::ResourceLocal( ::locate( "data", "knotes/notes.ics" ) );
  mCalendar->resourceManager()->add( mResource );
  mCalendar->load();

  connect( mCalendar, SIGNAL( calendarChanged() ), SLOT( updateView() ) );

  QPixmap icon = KGlobal::iconLoader()->loadIcon( "kontact_notes", KIcon::Desktop, KIcon::SizeMedium );
  QWidget* heading = createHeader( this, icon, i18n( "Notes" ) );

  mMainLayout->addWidget(heading);
  mLayout = new QVBoxLayout( mMainLayout );

  updateView();
}

void KNotesSummaryWidget::updateView()
{
  mNotes = mCalendar->journals();

  delete mLayout;
  mLayout = new QVBoxLayout( mMainLayout );

  mLabels.setAutoDelete( true );
  mLabels.clear();
  mLabels.setAutoDelete( false );

  KCal::Journal::List::Iterator it;
  for (it = mNotes.begin(); it != mNotes.end(); ++it) {
    KURLLabel *urlLabel = new KURLLabel(
        (*it)->uid(), (*it)->summary(), this );
    urlLabel->setTextFormat(RichText);
    urlLabel->show();
    mLayout->addWidget( urlLabel );
    mLabels.append( urlLabel );

    connect( urlLabel, SIGNAL( leftClickedURL( const QString& ) ),
        this, SLOT( urlClicked( const QString& ) ) );
  }
}

void KNotesSummaryWidget::urlClicked( const QString &/*uid*/ )
{
  if ( !mPlugin->isRunningStandalone() )
    mPlugin->core()->selectPlugin( mPlugin );
  else
    mPlugin->bringToForeground();
}

#include "summarywidget.moc"
