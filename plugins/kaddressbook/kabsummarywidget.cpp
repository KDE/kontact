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
  : QTextBrowser( parent, name )
{
  KABC::StdAddressBook *ab = KABC::StdAddressBook::self();
  connect( ab, SIGNAL( addressBookChanged( AddressBook* ) ),
           this, SLOT( updateView() ) );

  updateView();
}

void KABSummaryWidget::updateView()
{
  clear();

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

  QValueList<KABDateEntry>::Iterator addrIt;
  QString lines;
  for ( addrIt = dateList.begin(); addrIt != dateList.end(); ++addrIt ) {
    QString img = ( (*addrIt).birthday ? "birthday" : "anniversary" );
    QString date = KGlobal::locale()->formatDate( (*addrIt).date, true );
    lines += QString( "<tr><td><img src=\"%1\"></td><td>%2</td><td>%3 (%4)</td></tr>" )
                    .arg( img )
                    .arg( date )
                    .arg( (*addrIt).addressee.formattedName() )
                    .arg( (*addrIt).yearsOld );
  }
  QMimeSourceFactory::defaultFactory()->setPixmap( "birthday",
            KGlobal::iconLoader()->loadIcon( "cookie", KIcon::Small ) );
  QMimeSourceFactory::defaultFactory()->setPixmap( "anniversary",
            KGlobal::iconLoader()->loadIcon( "kuser", KIcon::Small ) );

  setText( QString( "<html><body>"
                    "<h1>%1</h1>"
                    "<table>%2</table>"
                    "</body></html>" ).arg( i18n( "Birthdays and Anniversaries" ) )
                                      .arg( lines ) );
}

#include "kabsummarywidget.moc"

