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

#include <qcursor.h>
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
#include <kpopupmenu.h>
#include <kurllabel.h>

#include "core.h"
#include "plugin.h"

#include "kabsummarywidget.h"

class KABDateEntry
{
  public:
    bool birthday;
    int yearsOld;
    int daysTo;
    QDate date;
    KABC::Addressee addressee;

    bool operator<( const KABDateEntry &entry ) const
    {
      return daysTo < entry.daysTo;
    }
};

KABSummaryWidget::KABSummaryWidget( Kontact::Plugin *plugin, QWidget *parent,
                                    const char *name )
  : Kontact::Summary( parent, name ), mPlugin( plugin )
{
  QVBoxLayout *mainLayout = new QVBoxLayout( this, 3, 3 );

  QPixmap icon = KGlobal::iconLoader()->loadIcon( "kontact_contacts",
                    KIcon::Desktop, KIcon::SizeMedium );

  QWidget *header = createHeader( this, icon, i18n( "Birthdays and Anniversaries" ) );
  mainLayout->addWidget(header);

  mLayout = new QGridLayout( mainLayout, 7, 5, 3 );

  KABC::StdAddressBook *ab = KABC::StdAddressBook::self( true );
  connect( ab, SIGNAL( addressBookChanged( AddressBook* ) ),
           this, SLOT( updateView() ) );

  connect( mPlugin->core(), SIGNAL( dayChanged( const QDate& ) ),
           this, SLOT( updateView() ) );

  configUpdated();
}

void KABSummaryWidget::configUpdated()
{
  KConfig config( "kcmkabsummaryrc" );

  config.setGroup( "Days" );
  mDaysAhead = config.readNumEntry( "DaysToShow", 7 );

  config.setGroup( "EventTypes" );
  mShowBirthdays = config.readBoolEntry( "ShowBirthdays", true );
  mShowAnniversaries = config.readBoolEntry( "ShowAnniversaries", true );

  updateView();
}

void KABSummaryWidget::updateView()
{
  mLabels.setAutoDelete( true );
  mLabels.clear();
  mLabels.setAutoDelete( false );

  KABC::StdAddressBook *ab = KABC::StdAddressBook::self( true );
  QValueList<KABDateEntry> dates;
  QLabel *label = 0;

  KABC::AddressBook::Iterator it;
  for ( it = ab->begin(); it != ab->end(); ++it ) {
    QDate birthday = (*it).birthday().date();
    if ( birthday.isValid() && mShowBirthdays ) {
      KABDateEntry entry;
      entry.birthday = true;
      dateDiff( birthday, entry.daysTo, entry.yearsOld );

      entry.date = birthday;
      entry.addressee = *it;
      if ( entry.daysTo <= mDaysAhead )
        dates.append( entry );
    }

    QString anniversaryAsString = (*it).custom( "KADDRESSBOOK" , "X-Anniversary" );
    if ( !anniversaryAsString.isEmpty() ) {
      QDate anniversary = QDate::fromString( anniversaryAsString , Qt::ISODate );
      if ( anniversary.isValid() && mShowAnniversaries ) {
         KABDateEntry entry;
         entry.birthday = false;
         dateDiff( anniversary, entry.daysTo, entry.yearsOld );

         entry.date = anniversary;
         entry.addressee = *it;
         if ( entry.daysTo <= mDaysAhead )
           dates.append( entry );
       }
    }
  }

  qHeapSort( dates );

  if ( !dates.isEmpty() ) {
    int counter = 0;
    QValueList<KABDateEntry>::Iterator addrIt;
    QString lines;
    for ( addrIt = dates.begin(); addrIt != dates.end(); ++addrIt ) {
      bool makeBold = (*addrIt).daysTo < 5;

      label = new QLabel( this );
      if ( (*addrIt).birthday )
        label->setPixmap( KGlobal::iconLoader()->loadIcon( "cookie", KIcon::Small ) );
      else
        label->setPixmap( KGlobal::iconLoader()->loadIcon( "kdmconfig", KIcon::Small ) );
      mLayout->addWidget( label, counter, 0 );
      mLabels.append( label );

      label = new QLabel( this );
      if ( (*addrIt).daysTo == 0 )
        label->setText( i18n( "Today" ) );
      else
        label->setText( i18n( "in 1 day", "in %n days", (*addrIt).daysTo ) );
      mLayout->addWidget( label, counter, 1 );
      mLabels.append( label );
      if ( makeBold ) {
        QFont font = label->font();
        font.setBold( true );
        label->setFont( font );
      }

      label = new QLabel( KGlobal::locale()->formatDate( (*addrIt).date, true ), this );
      mLayout->addWidget( label, counter, 2 );
      mLabels.append( label );

      KURLLabel *urlLabel = new KURLLabel( this );
      urlLabel->installEventFilter(this);
      urlLabel->setURL( (*addrIt).addressee.uid() );
      urlLabel->setText( (*addrIt).addressee.realName() );
      mLayout->addWidget( urlLabel, counter, 3 );
      mLabels.append( urlLabel );
      if ( makeBold ) {
        QFont font = label->font();
        font.setBold( true );
        label->setFont( font );
      }

      connect( urlLabel, SIGNAL( leftClickedURL( const QString& ) ),
          this, SLOT( mailContact( const QString& ) ) );
      connect( urlLabel, SIGNAL( rightClickedURL( const QString& ) ),
          this, SLOT( popupMenu( const QString& ) ) );

      label = new QLabel( this );
      label->setText( i18n( "one year", "%n years", (*addrIt).yearsOld  ) );
      mLayout->addWidget( label, counter, 4 );
      mLabels.append( label );
      if ( makeBold ) {
        QFont font = label->font();
        font.setBold( true );
        label->setFont( font );
      }

      counter++;
    }
  } else {
    label = new QLabel(
        i18n( "No birthdays or anniversaries pending within the next 1 day",
              "No birthdays or anniversaries pending within the next %n days",
              mDaysAhead ), this, "nothing to see" );
    label->setAlignment( AlignCenter );
    label->setTextFormat( RichText );
    mLabels.append( label );
    mLayout->addMultiCellWidget( label, 0, 0, 0, 4 );
  }

  for ( label = mLabels.first(); label; label = mLabels.next() )
    label->show();
}

void KABSummaryWidget::mailContact( const QString &uid )
{
  QString app;
  if ( kapp->dcopClient()->isApplicationRegistered( "kmail" ) )
    app = QString::fromLatin1( "kmail" );
  else {
    mPlugin->core()->selectPlugin( "mails" );
    app = QString::fromLatin1( "kontact" );
  }

  KABC::StdAddressBook *ab = KABC::StdAddressBook::self( true );
  QString email = ab->findByUid( uid ).fullEmail();

  // FIXME: replace "DCOPRef, dcopCall.send..." with kapp->invokeMailer for kde 3.2
  // kapp->invokeMailer(addr, QString::null);
  DCOPRef dcopCall( app.latin1(), "KMailIface" );
  dcopCall.send( "openComposer(QString,QString,QString,QString,QString,bool)", email,
                 QString::null, QString::null, QString::null, QString::null, false );
}

void KABSummaryWidget::viewContact( const QString &uid )
{
  if ( !mPlugin->isRunningStandalone() )
    mPlugin->core()->selectPlugin( mPlugin );
  else
    mPlugin->bringToForeground();

  DCOPRef dcopCall( "kaddressbook", "KAddressBookIface" );
  dcopCall.send( "showContactEditor(QString)", uid );
}

void KABSummaryWidget::popupMenu( const QString &uid )
{
  KPopupMenu popup( this );
  popup.insertItem( KGlobal::iconLoader()->loadIcon( "kontact_mail", KIcon::Small ),
                    i18n( "Send &Mail" ), 0 );
  popup.insertItem( KGlobal::iconLoader()->loadIcon( "kontact_contact", KIcon::Small ),
                    i18n( "View &Contact" ), 1 );

  switch ( popup.exec( QCursor::pos() ) ) {
    case 0:
      mailContact( uid );
      break;
    case 1:
      viewContact( uid );
      break;
  }
}

bool KABSummaryWidget::eventFilter( QObject *obj, QEvent* e )
{
  if ( obj->inherits( "KURLLabel" ) ) {
    KURLLabel* label = static_cast<KURLLabel*>( obj );
    if ( e->type() == QEvent::Enter )
      emit message( i18n( "Mail to %1" ).arg( label->text() ) );
    if ( e->type() == QEvent::Leave )
      emit message( QString::null );
  }

  return Kontact::Summary::eventFilter( obj, e );
}

void KABSummaryWidget::dateDiff( const QDate &date, int &days, int &years )
{
  QDate currentDate;
  QDate eventDate;

  if ( QDate::leapYear( date.year() ) && date.month() == 2 && date.day() == 29 ) {
    currentDate = QDate( date.year(), QDate::currentDate().month(), QDate::currentDate().day() );
    if ( !QDate::leapYear( QDate::currentDate().year() ) )
      eventDate = QDate( date.year(), date.month(), 28 ); // celebrate one day earlier ;)
    else
      eventDate = QDate( date.year(), date.month(), date.day() );
  } else {
    currentDate = QDate( 0, QDate::currentDate().month(), QDate::currentDate().day() );
    eventDate = QDate( 0, date.month(), date.day() );
  }

  int offset = currentDate.daysTo( eventDate );
  if ( offset < 0 ) {
    days = 365 + offset;
    years = QDate::currentDate().year() + 1 - date.year();
  } else {
    days = offset;
    years = QDate::currentDate().year() - date.year();
  }
}

QStringList KABSummaryWidget::configModules() const
{
  return QStringList( "kcmkabsummary.desktop" );
}

#include "kabsummarywidget.moc"
