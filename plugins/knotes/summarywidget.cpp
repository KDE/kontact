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

#include "summarywidget.h"

SummaryWidget::SummaryWidget( QWidget *parent, const char *name )
  : Kontact::Summary( parent, name )
{
  mMainLayout = new QVBoxLayout( this, 3, 3 );

  mICal = new KCal::CalendarLocal;

  // doesn't get triggered (danimo)
  connect(mICal, SIGNAL(calendarChanged()), SLOT(updateView()));

  QPixmap icon = KGlobal::iconLoader()->loadIcon( "knotes", KIcon::Desktop, KIcon::SizeMedium );
  QWidget* heading = createHeader( this, icon, i18n( "Notes" ) );

  mMainLayout->addWidget(heading);
  mLayout = new QVBoxLayout( mMainLayout );

  updateView();
}

bool SummaryWidget::ensureKNotesRunning()
{
  QString error;
  if ( !kapp->dcopClient()->isApplicationRegistered( "knotes" ) ) {
    if ( KApplication::startServiceByDesktopName( 
          "knotes", QStringList(), &error ) != 0 ) 
    {
      kdDebug() << error << endl;
      return false;
    }
  }
  return true;
}

void SummaryWidget::updateView()
{
  mICal->load(::locate("data", "knotes/notes.ics"));
  mNotes = mICal->journals();

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
    mLayout->addWidget( urlLabel );
    mLabels.append( urlLabel );

    connect( urlLabel, SIGNAL( leftClickedURL( const QString& ) ),
        this, SLOT( urlClicked( const QString& ) ) );
  }

  mLayout->addStretch();
}

void SummaryWidget::urlClicked( const QString &uid )
{
  if (ensureKNotesRunning())
  {
    DCOPRef dcopCall( "knotes", "KNotesIface" );
    dcopCall.send( "showNote(QString)", uid );
  }
}

#include "summarywidget.moc"
