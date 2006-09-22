/*
    This file is part of Kontact.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>
    Copyright (c) 2004 Allen Winter <winter@kde.org>

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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include <qcursor.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qimage.h>
#include <qtooltip.h>

#include <dcopclient.h>
#include <dcopref.h>
#include <kabc/stdaddressbook.h>
#include <korganizer/stdcalendar.h>
#include <kapplication.h>
#include <kdialog.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kparts/part.h>
#include <kpopupmenu.h>
#include <kstandarddirs.h>
#include <kurllabel.h>
#include <libkcal/event.h>
#include <libkcal/resourcecalendar.h>
#include <libkcal/resourcelocal.h>
#include <libkdepim/kpimprefs.h>

#include "core.h"
#include "plugin.h"

#include "sdsummarywidget.h"

enum SDIncidenceType {
  IncidenceTypeContact, IncidenceTypeEvent
};
enum SDCategory {
  CategoryBirthday, CategoryAnniversary, CategoryHoliday, CategoryOther
};

class SDEntry
{
  public:
    SDIncidenceType type;
    SDCategory category;
    int yearsOld;
    int daysTo;
    QDate date;
    QString summary;
    QString desc;
    int span; // #days in the special occassion.
    KABC::Addressee addressee;

    bool operator<( const SDEntry &entry ) const
    {
      return daysTo < entry.daysTo;
    }
};

SDSummaryWidget::SDSummaryWidget( Kontact::Plugin *plugin, QWidget *parent,
                                    const char *name )
  : Kontact::Summary( parent, name ), mPlugin( plugin ), mCalendar( 0 ), mHolidays( 0 )
{
  // Create the Summary Layout
  QVBoxLayout *mainLayout = new QVBoxLayout( this, 3, 3 );

  QPixmap icon = KGlobal::iconLoader()->loadIcon( "cookie",
                    KIcon::Desktop, KIcon::SizeMedium );

  QWidget *header = createHeader( this, icon, i18n( "Special Dates" ) );
  mainLayout->addWidget(header);

  mLayout = new QGridLayout( mainLayout, 7, 6, 3 );
  mLayout->setRowStretch( 6, 1 );

  // Setup the Addressbook
  KABC::StdAddressBook *ab = KABC::StdAddressBook::self( true );
  connect( ab, SIGNAL( addressBookChanged( AddressBook* ) ),
           this, SLOT( updateView() ) );
  connect( mPlugin->core(), SIGNAL( dayChanged( const QDate& ) ),
           this, SLOT( updateView() ) );

  // Setup the Calendar
  mCalendar = new KCal::CalendarResources( KPimPrefs::timezone() );
  mCalendar->readConfig();

  KCal::CalendarResourceManager *manager = mCalendar->resourceManager();
  if ( manager->isEmpty() ) {
    KConfig config( "korganizerrc" );
    config.setGroup( "General" );
    QString fileName = config.readPathEntry( "Active Calendar" );

    QString resourceName;
    if ( fileName.isEmpty() ) {
      fileName = locateLocal( "data", "korganizer/std.ics" );
      resourceName = i18n( "Default KOrganizer resource" );
    } else {
      resourceName = i18n( "Active Calendar" );
    }

    KCal::ResourceCalendar *defaultResource =
      new KCal::ResourceLocal( fileName );

    defaultResource->setResourceName( resourceName );

    manager->add( defaultResource );
    manager->setStandardResource( defaultResource );
  }
  mCalendar = KOrg::StdCalendar::self();
  mCalendar->load();

  connect( mCalendar, SIGNAL( calendarChanged() ),
           this, SLOT( updateView() ) );
  connect( mPlugin->core(), SIGNAL( dayChanged( const QDate& ) ),
           this, SLOT( updateView() ) );

  // Update Configuration
  configUpdated();
}

void SDSummaryWidget::configUpdated()
{
  KConfig config( "kcmsdsummaryrc" );

  config.setGroup( "Days" );
  mDaysAhead = config.readNumEntry( "DaysToShow", 7 );

  config.setGroup( "EventTypes" );
  mShowBirthdaysFromKAB =
    config.readBoolEntry( "ShowBirthdaysFromContacts", true );
  mShowBirthdaysFromCal =
    config.readBoolEntry( "ShowBirthdaysFromCalendar", true );

  mShowAnniversariesFromKAB =
    config.readBoolEntry( "ShowAnniversariesFromContacts", true );
  mShowAnniversariesFromCal =
    config.readBoolEntry( "ShowAnniversariesFromCalendar", true );

  mShowHolidays =
    config.readBoolEntry( "ShowHolidays", true );

  mShowSpecialsFromCal =
    config.readBoolEntry( "ShowSpecialsFromCalendar", true );

  updateView();
}

bool SDSummaryWidget::initHolidays()
{
  KConfig hconfig( "korganizerrc" );
  hconfig.setGroup( "Time & Date" );
  QString location = hconfig.readEntry( "Holidays" );
  if ( !location.isEmpty() ) {
    if ( mHolidays ) delete mHolidays;
    mHolidays = new KHolidays( location );
    return true;
  }
  return false;
}

// number of days remaining in an Event
int SDSummaryWidget::span( KCal::Event *event )
{
  int span=1;
  if ( event->isMultiDay() && event->doesFloat() ) {
    QDate d = event->dtStart().date();
    if ( d < QDate::currentDate() ) {
      d = QDate::currentDate();
    }
    while ( d < event->dtEnd().date() ) {
      span++;
      d=d.addDays( 1 );
    }
  }
  return span;
}

// day of a multiday Event
int SDSummaryWidget::dayof( KCal::Event *event, const QDate& date )
{
  int dayof=1;
  QDate d = event->dtStart().date();
  if ( d < QDate::currentDate() ) {
    d = QDate::currentDate();
  }
  while ( d < event->dtEnd().date() ) {
    if ( d < date ) {
      dayof++;
    }
    d = d.addDays( 1 );
  }
  return dayof;
}



void SDSummaryWidget::updateView()
{
  mLabels.setAutoDelete( true );
  mLabels.clear();
  mLabels.setAutoDelete( false );

  KABC::StdAddressBook *ab = KABC::StdAddressBook::self( true );
  QValueList<SDEntry> dates;
  QLabel *label = 0;

  // No reason to show the date year
  QString savefmt = KGlobal::locale()->dateFormat();
  KGlobal::locale()->setDateFormat( KGlobal::locale()->
                                    dateFormat().replace( 'Y', ' ' ) );

  // Search for Birthdays and Anniversaries in the Addressbook
  KABC::AddressBook::Iterator it;
  for ( it = ab->begin(); it != ab->end(); ++it ) {
    QDate birthday = (*it).birthday().date();
    if ( birthday.isValid() && mShowBirthdaysFromKAB ) {
      SDEntry entry;
      entry.type = IncidenceTypeContact;
      entry.category = CategoryBirthday;
      dateDiff( birthday, entry.daysTo, entry.yearsOld );

      entry.date = birthday;
      entry.addressee = *it;
      entry.span = 1;
      if ( entry.daysTo <= mDaysAhead )
        dates.append( entry );
    }

    QString anniversaryAsString =
      (*it).custom( "KADDRESSBOOK" , "X-Anniversary" );
    if ( !anniversaryAsString.isEmpty() ) {
      QDate anniversary = QDate::fromString( anniversaryAsString, Qt::ISODate );
      if ( anniversary.isValid() && mShowAnniversariesFromKAB ) {
        SDEntry entry;
        entry.type = IncidenceTypeContact;
        entry.category = CategoryAnniversary;
        dateDiff( anniversary, entry.daysTo, entry.yearsOld );

        entry.date = anniversary;
        entry.addressee = *it;
        entry.span = 1;
        if ( entry.daysTo <= mDaysAhead )
          dates.append( entry );
      }
    }
  }

  // Search for Birthdays, Anniversaries, Holidays, and Special Occasions
  // in the Calendar
  QDate dt;
  QDate currentDate = QDate::currentDate();
  for ( dt=currentDate;
        dt<=currentDate.addDays( mDaysAhead - 1 );
        dt=dt.addDays(1) ) {
    KCal::Event::List events = mCalendar->events( dt,
                                                  KCal::EventSortStartDate,
                                                  KCal::SortDirectionAscending );
    KCal::Event *ev;
    KCal::Event::List::ConstIterator it;
    for ( it=events.begin(); it!=events.end(); ++it ) {
      ev = *it;
      if ( !ev->categoriesStr().isEmpty() ) {
        QStringList::ConstIterator it2;
        QStringList c = ev->categories();
        for ( it2=c.begin(); it2!=c.end(); ++it2 ) {

          // Append Birthday Event?
          if ( mShowBirthdaysFromCal &&
               ( ( *it2 ).upper() == i18n( "BIRTHDAY" ) ) ) {
            SDEntry entry;
            entry.type = IncidenceTypeEvent;
            entry.category = CategoryBirthday;
            entry.date = dt;
            entry.summary = ev->summary();
            entry.desc = ev->description();
            dateDiff( ev->dtStart().date(), entry.daysTo, entry.yearsOld );
            entry.span = 1;
            dates.append( entry );
            break;
          }

          // Append Anniversary Event?
          if ( mShowAnniversariesFromCal &&
               ( ( *it2 ).upper() == i18n( "ANNIVERSARY" ) ) ) {
            SDEntry entry;
            entry.type = IncidenceTypeEvent;
            entry.category = CategoryAnniversary;
            entry.date = dt;
            entry.summary = ev->summary();
            entry.desc = ev->description();
            dateDiff( ev->dtStart().date(), entry.daysTo, entry.yearsOld );
            entry.span = 1;
            dates.append( entry );
            break;
          }

          // Append Holiday Event?
          if ( mShowHolidays &&
               ( ( *it2 ).upper() == i18n( "HOLIDAY" ) ) ) {
            SDEntry entry;
            entry.type = IncidenceTypeEvent;
            entry.category = CategoryHoliday;
            entry.date = dt;
            entry.summary = ev->summary();
            entry.desc = ev->description();
            dateDiff( dt, entry.daysTo, entry.yearsOld );
            entry.yearsOld = -1; //ignore age of holidays
            entry.span = span( ev );
            if ( entry.span > 1 && dayof( ev, dt ) > 1 ) // skip days 2,3,...
              break;
            dates.append( entry );
            break;
          }

          // Append Special Occasion Event?
          if ( mShowSpecialsFromCal &&
               ( ( *it2 ).upper() == i18n( "SPECIAL OCCASION" ) ) ) {
            SDEntry entry;
            entry.type = IncidenceTypeEvent;
            entry.category = CategoryOther;
            entry.date = dt;
            entry.summary = ev->summary();
            entry.desc = ev->description();
            dateDiff( dt, entry.daysTo, entry.yearsOld );
            entry.yearsOld = -1; //ignore age of special occasions
            entry.span = span( ev );
            if ( entry.span > 1 && dayof( ev, dt ) > 1 ) // skip days 2,3,...
              break;
            dates.append( entry );
            break;
          }
        }
      }
    }
  }

  // Seach for Holidays
  if ( mShowHolidays ) {
    if ( initHolidays() ) {
      for ( dt=currentDate;
            dt<=currentDate.addDays( mDaysAhead - 1 );
            dt=dt.addDays(1) ) {
        QValueList<KHoliday> holidays = mHolidays->getHolidays( dt );
        QValueList<KHoliday>::ConstIterator it = holidays.begin();
        for ( ; it != holidays.end(); ++it ) {
          SDEntry entry;
          entry.type = IncidenceTypeEvent;
          entry.category = ((*it).Category==KHolidays::HOLIDAY)?CategoryHoliday:CategoryOther;
          entry.date = dt;
          entry.summary = (*it).text;
          dateDiff( dt, entry.daysTo, entry.yearsOld );
          entry.yearsOld = -1; //ignore age of holidays
          entry.span = 1;
          dates.append( entry );
        }
      }
    }
  }

  // Sort, then Print the Special Dates
  qHeapSort( dates );

  if ( !dates.isEmpty() ) {
    int counter = 0;
    QValueList<SDEntry>::Iterator addrIt;
    QString lines;
    for ( addrIt = dates.begin(); addrIt != dates.end(); ++addrIt ) {
      bool makeBold = (*addrIt).daysTo == 0; // i.e., today

      // Pixmap
      QImage icon_img;
      QString icon_name;
      KABC::Picture pic;
      switch( (*addrIt).category ) {  // TODO: better icons
      case CategoryBirthday:
        icon_name = "cookie";
        pic = (*addrIt).addressee.photo();
        if ( pic.isIntern() && !pic.data().isNull() ) {
          QImage img = pic.data();
          if ( img.width() > img.height() ) {
            icon_img = img.scaleWidth( 32 );
          } else {
            icon_img = img.scaleHeight( 32 );
          }
        }
        break;
      case CategoryAnniversary:
        icon_name = "kdmconfig";
        pic = (*addrIt).addressee.photo();
        if ( pic.isIntern() && !pic.data().isNull() ) {
          QImage img = pic.data();
          if ( img.width() > img.height() ) {
            icon_img = img.scaleWidth( 32 );
          } else {
            icon_img = img.scaleHeight( 32 );
          }
        }
        break;
      case CategoryHoliday:
        icon_name = "kdmconfig"; break;
      case CategoryOther:
        icon_name = "cookie"; break;
      }
      label = new QLabel( this );
      if ( icon_img.isNull() ) {
        label->setPixmap( KGlobal::iconLoader()->loadIcon( icon_name,
                                                           KIcon::Small ) );
      } else {
        label->setPixmap( icon_img );
      }
      label->setMaximumWidth( label->minimumSizeHint().width() );
      label->setAlignment( AlignVCenter );
      mLayout->addWidget( label, counter, 0 );
      mLabels.append( label );

      // Event date
      QString datestr;

      //Muck with the year -- change to the year 'daysTo' days away
      int year = currentDate.addDays( (*addrIt).daysTo ).year();
      QDate sD = QDate::QDate( year,
                           (*addrIt).date.month(), (*addrIt).date.day() );

      if ( (*addrIt).daysTo == 0 ) {
        datestr = i18n( "Today" );
      } else if ( (*addrIt).daysTo == 1 ) {
        datestr = i18n( "Tomorrow" );
      } else {
        datestr = KGlobal::locale()->formatDate( sD );
      }
      // Print the date span for multiday, floating events, for the
      // first day of the event only.
      if ( (*addrIt).span > 1 ) {
        QString endstr =
          KGlobal::locale()->formatDate( sD.addDays( (*addrIt).span - 1 ) );
        datestr += " -\n " + endstr;
      }

      label = new QLabel( datestr, this );
      label->setAlignment( AlignLeft | AlignVCenter );
      mLayout->addWidget( label, counter, 1 );
      mLabels.append( label );
      if ( makeBold ) {
        QFont font = label->font();
        font.setBold( true );
        label->setFont( font );
      }

      // Countdown
      label = new QLabel( this );
      if ( (*addrIt).daysTo == 0 ) {
        label->setText( i18n( "now" ) );
      } else {
        label->setText( i18n( "in 1 day", "in %n days", (*addrIt).daysTo ) );
      }

      label->setAlignment( AlignLeft | AlignVCenter );
      mLayout->addWidget( label, counter, 2 );
      mLabels.append( label );

      // What
      QString what;
      switch( (*addrIt).category ) {
      case CategoryBirthday:
        what = i18n( "Birthday" ); break;
      case CategoryAnniversary:
        what = i18n( "Anniversary" ); break;
      case CategoryHoliday:
        what = i18n( "Holiday" ); break;
      case CategoryOther:
        what = i18n( "Special Occasion" ); break;
      }
      label = new QLabel( this );
      label->setText( what );
      label->setAlignment( AlignLeft | AlignVCenter );
      mLayout->addWidget( label, counter, 3 );
      mLabels.append( label );

      // Description
      if ( (*addrIt).type == IncidenceTypeContact ) {
        KURLLabel *urlLabel = new KURLLabel( this );
        urlLabel->installEventFilter( this );
        urlLabel->setURL( (*addrIt).addressee.uid() );
        urlLabel->setText( (*addrIt).addressee.realName() );
        urlLabel->setTextFormat( Qt::RichText );
        mLayout->addWidget( urlLabel, counter, 4 );
        mLabels.append( urlLabel );

        connect( urlLabel, SIGNAL( leftClickedURL( const QString& ) ),
                 this, SLOT( mailContact( const QString& ) ) );
        connect( urlLabel, SIGNAL( rightClickedURL( const QString& ) ),
                 this, SLOT( popupMenu( const QString& ) ) );
      } else {
        label = new QLabel( this );
        label->setText( (*addrIt).summary );
        label->setTextFormat( Qt::RichText );
        mLayout->addWidget( label, counter, 4 );
        mLabels.append( label );
        if ( !(*addrIt).desc.isEmpty() ) {
          QToolTip::add( label, (*addrIt).desc );
        }
      }

     // Age
      if ( (*addrIt).category == CategoryBirthday ||
           (*addrIt).category == CategoryAnniversary ) {
        label = new QLabel( this );
        if ( (*addrIt).yearsOld <= 0 ) {
          label->setText( "" );
        } else {
          label->setText( i18n( "one year", "%n years", (*addrIt).yearsOld  ) );
        }
        label->setAlignment( AlignLeft | AlignVCenter );
        mLayout->addWidget( label, counter, 5 );
        mLabels.append( label );
      }

      counter++;
    }
  } else {
    label = new QLabel(
        i18n( "No special dates within the next 1 day",
              "No special dates pending within the next %n days",
              mDaysAhead ), this, "nothing to see" );
    label->setAlignment( AlignHCenter | AlignVCenter );
    mLayout->addMultiCellWidget( label, 0, 0, 0, 4 );
    mLabels.append( label );
  }

  for ( label = mLabels.first(); label; label = mLabels.next() )
    label->show();

  KGlobal::locale()->setDateFormat( savefmt );
}

void SDSummaryWidget::mailContact( const QString &uid )
{
  KABC::StdAddressBook *ab = KABC::StdAddressBook::self( true );
  QString email = ab->findByUid( uid ).fullEmail();

  kapp->invokeMailer( email, QString::null );
}

void SDSummaryWidget::viewContact( const QString &uid )
{
  if ( !mPlugin->isRunningStandalone() )
    mPlugin->core()->selectPlugin( "kontact_kaddressbookplugin" );
  else
    mPlugin->bringToForeground();

  DCOPRef dcopCall( "kaddressbook", "KAddressBookIface" );
  dcopCall.send( "showContactEditor(QString)", uid );
}

void SDSummaryWidget::popupMenu( const QString &uid )
{
  KPopupMenu popup( this );
  popup.insertItem( KGlobal::iconLoader()->loadIcon( "kmail", KIcon::Small ),
                    i18n( "Send &Mail" ), 0 );
  popup.insertItem( KGlobal::iconLoader()->loadIcon( "kaddressbook", KIcon::Small ),
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

bool SDSummaryWidget::eventFilter( QObject *obj, QEvent* e )
{
  if ( obj->inherits( "KURLLabel" ) ) {
    KURLLabel* label = static_cast<KURLLabel*>( obj );
    if ( e->type() == QEvent::Enter )
      emit message( i18n( "Mail to:\"%1\"" ).arg( label->text() ) );
    if ( e->type() == QEvent::Leave )
      emit message( QString::null );
  }

  return Kontact::Summary::eventFilter( obj, e );
}

void SDSummaryWidget::dateDiff( const QDate &date, int &days, int &years )
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

QStringList SDSummaryWidget::configModules() const
{
  return QStringList( "kcmsdsummary.desktop" );
}

#include "sdsummarywidget.moc"
