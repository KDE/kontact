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
  QMap<QDate, KABC::Addressee> prevDates;
  QMap<QDate, KABC::Addressee> nextDates;

  QDate currentDate( 0, QDate::currentDate().month(),
                     QDate::currentDate().day() );

  KABC::AddressBook::Iterator it;
  for ( it = ab->begin(); it != ab->end(); ++it ) {
    QDate d = (*it).birthday().date();
    if ( d.isValid() ) {
      QDate date( 0, d.month(), d.day() );
      if ( date < currentDate )
        prevDates.insert( date, *it, false );
      else
        nextDates.insert( date, *it, false );
    }
  }

  KABC::Addressee::List addrList;
  QMap<QDate, KABC::Addressee>::Iterator dateIt;
  for ( dateIt = nextDates.begin(); dateIt != nextDates.end(); ++dateIt )
    addrList.append( dateIt.data() );

  for ( dateIt = prevDates.begin(); dateIt != prevDates.end(); ++dateIt )
    addrList.append( dateIt.data() );

  KABC::Addressee::List::Iterator addrIt;
  QString lines;
  for ( addrIt = addrList.begin(); addrIt != addrList.end(); ++addrIt ) {
    lines += QString( "<tr><td><img src=\"%1\"></td><td>%2</td><td>%3</td></tr>" )
                    .arg( "birthday" )
                    .arg( KGlobal::locale()->formatDate( 
                          (*addrIt).birthday().date(), true ) )
                    .arg( (*addrIt).formattedName() );
  }
  QMimeSourceFactory::defaultFactory()->setPixmap( "birthday",
            KGlobal::iconLoader()->loadIcon( "bookmark", KIcon::Small ) );

  setText( QString( "<html><body><table>%1</table></body></html>" ).arg( lines ) );
}

#include "kabsummarywidget.moc"

