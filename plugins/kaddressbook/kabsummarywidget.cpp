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
#include <kabc/stdaddressbook.h>
#include <kapplication.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kparts/part.h>
#include <kurllabel.h>

#include "core.h"
#include "plugin.h"

#include "kabsummarywidget.h"

class KABDateEntry
{
  public:
    bool birthday;
    int yearsOld;
    QDate date;
    KABC::Addressee addressee;

    bool operator<( const KABDateEntry &entry ) const
    {
      QDate thisDate( 0, date.month(), date.day() );
      QDate otherDate( 0, entry.date.month(), entry.date.day() );
      return thisDate < otherDate;
    }
};

KABSummaryWidget::KABSummaryWidget( Kontact::Plugin *plugin, QWidget *parent,
                                    const char *name )
  : Kontact::Summary( parent, name ), mPlugin( plugin )
{
  QVBoxLayout *mainLayout = new QVBoxLayout( this, 3, 3 );
  QHBoxLayout *hbox = new QHBoxLayout( mainLayout, 3 );

  QFont boldFont;
  boldFont.setBold( true );
  boldFont.setPointSize( boldFont.pointSize() + 2 );

  QLabel *label = new QLabel( this );
  label->setFixedSize( 32, 32 );
  label->setPixmap( KGlobal::iconLoader()->loadIcon( "kaddressbook", 
                    KIcon::Desktop, KIcon::SizeMedium ) );
  hbox->addWidget( label );

  label = new QLabel( i18n( "Birthdays and Anniversaries" ), this );
  label->setAlignment( AlignLeft );
  label->setFont( boldFont );
  hbox->addWidget( label );

  mLayout = new QGridLayout( mainLayout, 7, 4, 3 );
  mLayout->setRowStretch( 6, 1 );

  KABC::StdAddressBook *ab = KABC::StdAddressBook::self();
  connect( ab, SIGNAL( addressBookChanged( AddressBook* ) ),
           this, SLOT( updateView() ) );

  QString error;
  QCString appID;

  if ( kapp->dcopClient()->isApplicationRegistered( "kaddressbook" ) )
    mDCOPApp = "kaddressbook";
  else {
    plugin->part();
    mDCOPApp = "kontact";
  }

  updateView();
}

void KABSummaryWidget::updateView()
{
  mLabels.setAutoDelete( true );
  mLabels.clear();
  mLabels.setAutoDelete( false );

  KABC::StdAddressBook *ab = KABC::StdAddressBook::self();
  QValueList<KABDateEntry> prevDates;
  QValueList<KABDateEntry> nextDates;

  QDate currentDate( 0, QDate::currentDate().month(),
                     QDate::currentDate().day() );

  KABC::AddressBook::Iterator it;
  for ( it = ab->begin(); it != ab->end(); ++it ) {
    QDate birthday = (*it).birthday().date();
    QDate anniversary = QDate::fromString(
          (*it).custom( "KADDRESSBOOK" , "X-Anniversary" ), Qt::ISODate );
    if ( birthday.isValid() ) {
      QDate date( 0, birthday.month(), birthday.day() );
      KABDateEntry entry;
      entry.birthday = true;
      entry.yearsOld = QDate::currentDate().year() - birthday.year();
      entry.date = birthday;
      entry.addressee = *it;
      if ( date < currentDate )
        prevDates.append( entry );
      else
        nextDates.append( entry );
    }
    if ( anniversary.isValid() ) {
      QDate date( 0, anniversary.month(), anniversary.day() );
      KABDateEntry entry;
      entry.birthday = false;
      entry.yearsOld = QDate::currentDate().year() - anniversary.year();
      entry.date = anniversary;
      entry.addressee = *it;
      if ( date < currentDate )
        prevDates.append( entry );
      else
        nextDates.append( entry );
    }
  }

  qHeapSort( prevDates );
  qHeapSort( nextDates );

  QValueList<KABDateEntry> dateList;
  QValueList<KABDateEntry>::Iterator dateIt;
  for ( dateIt = nextDates.begin(); dateIt != nextDates.end(); ++dateIt )
    dateList.append( *dateIt );

  for ( dateIt = prevDates.begin(); dateIt != prevDates.end(); ++dateIt ) {
    (*dateIt).yearsOld++;
    dateList.append( *dateIt );
  }

  int counter = 0;
  QValueList<KABDateEntry>::Iterator addrIt;
  QString lines;
  for ( addrIt = dateList.begin(); addrIt != dateList.end() && counter < 6; ++addrIt ) {
    QLabel *label = new QLabel( this );
    if ( (*addrIt).birthday )
      label->setPixmap( KGlobal::iconLoader()->loadIcon( "cookie", KIcon::Small ) );
    else
      label->setPixmap( KGlobal::iconLoader()->loadIcon( "kdmconfig", KIcon::Small ) );
    mLayout->addWidget( label, counter, 0 );
    mLabels.append( label );

    label = new QLabel( KGlobal::locale()->formatDate( (*addrIt).date, true ), this );
    mLayout->addWidget( label, counter, 1 );
    mLabels.append( label );

    KURLLabel *urlLabel = new KURLLabel( this );
    urlLabel->setURL( (*addrIt).addressee.uid() );
    urlLabel->setText( (*addrIt).addressee.formattedName() );
    mLayout->addWidget( urlLabel, counter, 2 );
    mLabels.append( urlLabel );

    connect( urlLabel, SIGNAL( leftClickedURL( const QString& ) ),
             this, SLOT( selectContact( const QString& ) ) );

    label = new QLabel( this );
    label->setText( i18n( "one year", "%n years", (*addrIt).yearsOld  ) );
    mLayout->addWidget( label, counter, 3 );
    mLabels.append( label );

    counter++;
  }

  show();
}

void KABSummaryWidget::selectContact( const QString &uid )
{
  if ( mDCOPApp == "kontact" )
    mPlugin->core()->selectPlugin( mPlugin );

  DCOPRef dcopCall( mDCOPApp.latin1(), "KAddressBookIface" );
  dcopCall.send( "showContactEditor(QString)", uid );
}

#include "kabsummarywidget.moc"
