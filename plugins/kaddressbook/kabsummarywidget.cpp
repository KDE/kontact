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

#include <kabc/stdaddressbook.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>

#include "kabsummarywidget.h"

struct KABDateEntry
{
  bool birthday;
  int yearsOld;
  QDate date;
  KABC::Addressee addressee;
};

KABSummaryWidget::KABSummaryWidget( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  setPaletteBackgroundColor( QColor( 240, 240, 240 ) );

  mLayout = new QGridLayout( this, 7, 3, 3 );

  QFont boldFont;
  boldFont.setBold( true );

  QLabel *label = new QLabel( i18n( "Birthdays and Anniversaries" ), this );
  label->setFont( boldFont );
  mLayout->addMultiCellWidget( label, 0, 0, 0, 2 );

  KABC::StdAddressBook *ab = KABC::StdAddressBook::self();
  connect( ab, SIGNAL( addressBookChanged( AddressBook* ) ),
           this, SLOT( updateView() ) );

  updateView();
}

void KABSummaryWidget::updateView()
{
  mLabels.setAutoDelete( true );
  mLabels.clear();
  mLabels.setAutoDelete( false );

  KABC::StdAddressBook *ab = KABC::StdAddressBook::self();
  QMap<QDate, KABDateEntry> prevDates;
  QMap<QDate, KABDateEntry> nextDates;

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
        prevDates.insert( date, entry, false );
      else
        nextDates.insert( date, entry, false );
    }
    if ( anniversary.isValid() ) {
      QDate date( 0, anniversary.month(), anniversary.day() );
      KABDateEntry entry;
      entry.birthday = false;
      entry.yearsOld = QDate::currentDate().year() - anniversary.year();
      entry.date = anniversary;
      entry.addressee = *it;
      if ( date < currentDate )
        prevDates.insert( date, entry, false );
      else
        nextDates.insert( date, entry, false );
    }
  }

  QValueList<KABDateEntry> dateList;
  QMap<QDate, KABDateEntry>::Iterator dateIt;
  for ( dateIt = nextDates.begin(); dateIt != nextDates.end(); ++dateIt )
    dateList.append( dateIt.data() );

  for ( dateIt = prevDates.begin(); dateIt != prevDates.end(); ++dateIt ) {
    dateIt.data().yearsOld++;
    dateList.append( dateIt.data() );
  }

  int counter = 1;
  QValueList<KABDateEntry>::Iterator addrIt;
  QString lines;
  for ( addrIt = dateList.begin(); addrIt != dateList.end() && counter < 7; ++addrIt ) {
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

    label = new QLabel( this );
    label->setText( QString( "%1 (%2)" ).arg( (*addrIt).addressee.formattedName() )
                                        .arg( i18n( "one year", "%n years", (*addrIt).yearsOld  ) ) );
    mLayout->addWidget( label, counter, 2 );
    mLabels.append( label );

    counter++;
  }

  show();
}

#include "kabsummarywidget.moc"

