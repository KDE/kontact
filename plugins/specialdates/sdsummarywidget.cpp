/*
  This file is part of Kontact.

  Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>
  Copyright (c) 2004,2009 Allen Winter <winter@kde.org>

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

#include "sdsummarywidget.h"
#include "../korganizer/stdcalendar.h"

#include <akonadi/contact/contactsearchjob.h>
#include <akonadi/contact/contactviewerdialog.h>
#include <akonadi/itemfetchjob.h>
#include <akonadi/itemfetchscope.h>

#include <KCal/Calendar>
#include <KCal/CalHelper>

#include <KHolidays/Holidays>

#include <KontactInterface/Core>
#include <KontactInterface/Plugin>

#include <KConfigGroup>
#include <KIconLoader>
#include <KMenu>
#include <KToolInvocation>
#include <KUrlLabel>

#include <QDate>
#include <QEvent>
#include <QGridLayout>
#include <QLabel>

class BirthdaySearchJob : public Akonadi::ItemSearchJob
{
  public:
    BirthdaySearchJob( QObject *parent, int daysInAdvance );
};

BirthdaySearchJob::BirthdaySearchJob( QObject *parent, int daysInAdvance )
  : ItemSearchJob( QString(), parent )
{
  fetchScope().fetchFullPayload();
  const QString query = QString::fromLatin1(
      "prefix nco:<http://www.semanticdesktop.org/ontologies/2007/03/22/nco#> "
      "prefix xsd:<http://www.w3.org/2001/XMLSchema#> "
      ""
      "SELECT ?r "
      "WHERE { "
      "  graph ?g { "
      "    ?r a nco:PersonContact . "
      "    ?r <%1> ?akonadiItemId . "
      "    ?r nco:birthDate ?birthDate . "
      "    FILTER( bif:dayofyear(?birthDate) >= bif:dayofyear(xsd:date(\"%2\")) ) "
      "    FILTER( bif:dayofyear(?birthDate) <= bif:dayofyear(xsd:date(\"%2\")) + %3 ) "
      "  } "
      "}"
      ).arg( QString::fromLatin1( Akonadi::ItemSearchJob::akonadiItemIdUri().toEncoded() ) )
     .arg( QDate::currentDate().toString( Qt::ISODate ) )
     .arg( daysInAdvance );
  ItemSearchJob::setQuery( query );
}

enum SDIncidenceType {
  IncidenceTypeContact,
  IncidenceTypeEvent
};
enum SDCategory {
  CategoryBirthday,
  CategoryAnniversary,
  CategoryHoliday,
  CategoryOther
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
    Akonadi::Item item;

    bool operator<( const SDEntry &entry ) const
    {
      return daysTo < entry.daysTo;
    }
};

SDSummaryWidget::SDSummaryWidget( KontactInterface::Plugin *plugin, QWidget *parent )
  : KontactInterface::Summary( parent ), mPlugin( plugin ), mCalendar( 0 ), mHolidays( 0 )
{
  // Create the Summary Layout
  QVBoxLayout *mainLayout = new QVBoxLayout( this );
  mainLayout->setSpacing( 3 );
  mainLayout->setMargin( 3 );

  //TODO: we want our own special dates icon
  QWidget *header = createHeader(
    this, "favorites", i18n( "Upcoming Special Dates" ) );
  mainLayout->addWidget( header );

  mLayout = new QGridLayout();
  mainLayout->addItem( mLayout );
  mLayout->setSpacing( 3 );
  mLayout->setRowStretch( 6, 1 );

  // Default settings
  mDaysAhead = 7;
  mShowBirthdaysFromKAB = true;
  mShowBirthdaysFromCal = true;
  mShowAnniversariesFromKAB = true;
  mShowAnniversariesFromCal = true;
  mShowHolidays = true;
  mShowSpecialsFromCal = true;

  // Setup the Addressbook
  connect( mPlugin->core(), SIGNAL(dayChanged(const QDate&)),
           this, SLOT(updateView()) );

  // Setup the Calendar
  mCalendar = KOrg::StdCalendar::self();
  mCalendar->load();

  connect( mCalendar, SIGNAL(calendarChanged()),
           this, SLOT(updateView()) );
  connect( mPlugin->core(), SIGNAL(dayChanged(const QDate&)),
           this, SLOT(updateView()) );

  // Update Configuration
  configUpdated();
}

void SDSummaryWidget::configUpdated()
{
  KConfig config( "kcmsdsummaryrc" );

  KConfigGroup group = config.group( "Days" );
  mDaysAhead = group.readEntry( "DaysToShow", 7 );

  group = config.group( "Show" );
  mShowBirthdaysFromKAB = group.readEntry( "BirthdaysFromContacts", true );
  mShowBirthdaysFromCal = group.readEntry( "BirthdaysFromCalendar", true );

  mShowAnniversariesFromKAB = group.readEntry( "AnniversariesFromContacts", true );
  mShowAnniversariesFromCal = group.readEntry( "AnniversariesFromCalendar", true );

  mShowHolidays = group.readEntry( "HolidaysFromCalendar", true );

  mShowSpecialsFromCal = group.readEntry( "SpecialsFromCalendar", true );

  group = config.group( "Groupware" );
  mShowMineOnly = group.readEntry( "ShowMineOnly", false );

  updateView();
}

bool SDSummaryWidget::initHolidays()
{
  KConfig _hconfig( "korganizerrc" );
  KConfigGroup hconfig( &_hconfig, "Time & Date" );
  QString location = hconfig.readEntry( "Holidays" );
  if ( !location.isEmpty() ) {
    if ( mHolidays ) {
      delete mHolidays;
    }
    mHolidays = new HolidayRegion( location );
    return true;
  }
  return false;
}

// number of days remaining in an Event
int SDSummaryWidget::span( Event *event )
{
  int span=1;
  if ( event->isMultiDay() && event->allDay() ) {
    QDate d = event->dtStart().date();
    if ( d < QDate::currentDate() ) {
      d = QDate::currentDate();
    }
    while ( d < event->dtEnd().date() ) {
      span++;
      d = d.addDays( 1 );
    }
  }
  return span;
}

// day of a multiday Event
int SDSummaryWidget::dayof( Event *event, const QDate &date )
{
  int dayof = 1;
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
  KIconLoader loader( "kdepim" );

  QList<SDEntry> dates;
  QLabel *label = 0;

  // Remove all special date labels from the layout and delete them, as we
  // will re-create all labels below.
  setUpdatesEnabled( false );
  foreach ( label, mLabels ) {
    mLayout->removeWidget( label );
    delete( label );
  }
  mLabels.clear();

#if 0 // TODO: Port me!
  // Search for Anniversaries
  // <same code as in the birthday case here>
  const QString anniversaryAsString = contact.custom( "KADDRESSBOOK", "X-Anniversary" );
  if ( !anniversaryAsString.isEmpty() ) {
    const QDate anniversary = QDate::fromString( anniversaryAsString, Qt::ISODate );
    if ( anniversary.isValid() && mShowAnniversariesFromKAB ) {
      SDEntry entry;
      entry.type = IncidenceTypeContact;
      entry.category = CategoryAnniversary;
      dateDiff( anniversary, entry.daysTo, entry.yearsOld );

      entry.date = anniversary;
      entry.addressee = contact;
      entry.item = item;
      entry.span = 1;
      if ( entry.daysTo <= mDaysAhead ) {
        dates.append( entry );
      }
    }
  }

#else
  kWarning() << "Disabled code for anniversary searching, needs new S.D.O release";
#endif

  // Search for Birthdays
  if ( mShowBirthdaysFromKAB ) {
    BirthdaySearchJob *job = new BirthdaySearchJob( this, mDaysAhead );
    job->exec();

    foreach ( const Akonadi::Item &item, job->items() ) {
      if ( item.hasPayload<KABC::Addressee>() ) {
        const KABC::Addressee addressee = item.payload<KABC::Addressee>();
        const QDate birthday = addressee.birthday().date();
        if ( birthday.isValid() ) {
          SDEntry entry;
          entry.type = IncidenceTypeContact;
          entry.category = CategoryBirthday;
          dateDiff( birthday, entry.daysTo, entry.yearsOld );

          entry.date = birthday;
          entry.addressee = addressee;
          entry.item = item;
          entry.span = 1;
          dates.append( entry );
        }
      }
    }
  }

#if 0 //sebsauer
  // Search for Birthdays, Anniversaries, Holidays, and Special Occasions
  // in the Calendar
  ResourceCalendar *bdayRes = usingBirthdayResource();
  ResourceCalendar *annvRes = bdayRes;
  if ( !mShowBirthdaysFromKAB ) {
    bdayRes = 0;
  }
  if ( !mShowAnniversariesFromKAB ) {
    annvRes = 0;
  }
#else
  ResourceCalendar *bdayRes = 0;
  ResourceCalendar *annvRes = 0;
  kDebug() << "Disabled code";
#endif

  QDate dt;
  for ( dt=QDate::currentDate();
        dt<=QDate::currentDate().addDays( mDaysAhead - 1 );
        dt=dt.addDays(1) ) {
    Event::List events = mCalendar->events( dt, mCalendar->timeSpec(),
                                            EventSortStartDate,
                                            SortDirectionAscending );
    Event *ev;
    Event::List::ConstIterator it;
    for ( it = events.constBegin(); it != events.constEnd(); ++it ) {
      ev = *it;

      // Optionally, show only my Events
      if ( mShowMineOnly && !CalHelper::isMyCalendarIncidence( mCalendar, ev ) ) {
        continue;
      }

      if ( !ev->categoriesStr().isEmpty() ) {
        QStringList::ConstIterator it2;
        QStringList c = ev->categories();
        for ( it2 = c.constBegin(); it2 != c.constEnd(); ++it2 ) {

          // Append Birthday Event?
          if ( mShowBirthdaysFromCal && ( ( *it2 ).toUpper() == i18n( "BIRTHDAY" ) ) ) {
            SDEntry entry;
            entry.type = IncidenceTypeEvent;
            entry.category = CategoryBirthday;
            entry.date = dt;
            entry.summary = ev->summary();
            entry.desc = ev->description();
            dateDiff( ev->dtStart().date(), entry.daysTo, entry.yearsOld );
            entry.span = 1;
            if ( !check( bdayRes, dt, ev->summary() ) ) {
              dates.append( entry );
            }
            break;
          }

          // Append Anniversary Event?
          if ( mShowAnniversariesFromCal && ( ( *it2 ).toUpper() == i18n( "ANNIVERSARY" ) ) ) {
            SDEntry entry;
            entry.type = IncidenceTypeEvent;
            entry.category = CategoryAnniversary;
            entry.date = dt;
            entry.summary = ev->summary();
            entry.desc = ev->description();
            dateDiff( ev->dtStart().date(), entry.daysTo, entry.yearsOld );
            entry.span = 1;
            if ( !check( annvRes, dt, ev->summary() ) ) {
              dates.append( entry );
            }
            break;
          }

          // Append Holiday Event?
          if ( mShowHolidays && ( ( *it2 ).toUpper() == i18n( "HOLIDAY" ) ) ) {
            SDEntry entry;
            entry.type = IncidenceTypeEvent;
            entry.category = CategoryHoliday;
            entry.date = dt;
            entry.summary = ev->summary();
            entry.desc = ev->description();
            dateDiff( dt, entry.daysTo, entry.yearsOld );
            entry.yearsOld = -1; //ignore age of holidays
            entry.span = span( ev );
            if ( entry.span > 1 && dayof( ev, dt ) > 1 ) { // skip days 2,3,...
              break;
            }
            dates.append( entry );
            break;
          }

          // Append Special Occasion Event?
          if ( mShowSpecialsFromCal && ( ( *it2 ).toUpper() == i18n( "SPECIAL OCCASION" ) ) ) {
            SDEntry entry;
            entry.type = IncidenceTypeEvent;
            entry.category = CategoryOther;
            entry.date = dt;
            entry.summary = ev->summary();
            entry.desc = ev->description();
            dateDiff( dt, entry.daysTo, entry.yearsOld );
            entry.yearsOld = -1; //ignore age of special occasions
            entry.span = span( ev );
            if ( entry.span > 1 && dayof( ev, dt ) > 1 ) { // skip days 2,3,...
              break;
            }
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
      for ( dt=QDate::currentDate();
            dt<=QDate::currentDate().addDays( mDaysAhead - 1 );
            dt=dt.addDays(1) ) {
        QList<Holiday> holidays = mHolidays->holidays( dt );
        QList<Holiday>::ConstIterator it = holidays.constBegin();
        for ( ; it != holidays.constEnd(); ++it ) {
          SDEntry entry;
          entry.type = IncidenceTypeEvent;
          entry.category = ( (*it).dayType() == Holiday::NonWorkday )?
                           CategoryHoliday : CategoryOther;
          entry.date = dt;
          entry.summary = (*it).text();
          dateDiff( dt, entry.daysTo, entry.yearsOld );
          entry.yearsOld = -1; //ignore age of holidays
          entry.span = 1;
          dates.append( entry );
        }
      }
    }
  }

  // Sort, then Print the Special Dates
  qSort( dates );

  if ( !dates.isEmpty() ) {
    int counter = 0;
    QList<SDEntry>::Iterator addrIt;
    QString lines;
    for ( addrIt = dates.begin(); addrIt != dates.end(); ++addrIt ) {
      bool makeBold = (*addrIt).daysTo == 0; // i.e., today

      // Pixmap
      QImage icon_img;
      QString icon_name;
      KABC::Picture pic;
      switch( (*addrIt).category ) {
      case CategoryBirthday:
        icon_name = "view-calendar-birthday";
        pic = (*addrIt).addressee.photo();
        if ( pic.isIntern() && !pic.data().isNull() ) {
          QImage img = pic.data();
          if ( img.width() > img.height() ) {
            icon_img = img.scaledToWidth( 32 );
          } else {
            icon_img = img.scaledToHeight( 32 );
          }
        }
        break;
      case CategoryAnniversary:
        icon_name = "view-calendar-wedding-anniversary";
        pic = (*addrIt).addressee.photo();
        if ( pic.isIntern() && !pic.data().isNull() ) {
          QImage img = pic.data();
          if ( img.width() > img.height() ) {
            icon_img = img.scaledToWidth( 32 );
          } else {
            icon_img = img.scaledToHeight( 32 );
          }
        }
        break;
      case CategoryHoliday:
        icon_name = "view-calendar-holiday";
        break;
      case CategoryOther:
        icon_name = "favorites";
        break;
      }
      label = new QLabel( this );
      if ( icon_img.isNull() ) {
        label->setPixmap( KIconLoader::global()->loadIcon( icon_name, KIconLoader::Small ) );
      } else {
        label->setPixmap( QPixmap::fromImage( icon_img ) );
      }
      label->setMaximumWidth( label->minimumSizeHint().width() );
      label->setAlignment( Qt::AlignVCenter );
      mLayout->addWidget( label, counter, 0 );
      mLabels.append( label );

      // Event date
      QString datestr;

      //Muck with the year -- change to the year 'daysTo' days away
      int year = QDate::currentDate().addDays( (*addrIt).daysTo ).year();
      QDate sD = QDate( year, (*addrIt).date.month(), (*addrIt).date.day() );

      if ( (*addrIt).daysTo == 0 ) {
        datestr = i18nc( "the special day is today", "Today" );
      } else if ( (*addrIt).daysTo == 1 ) {
        datestr = i18nc( "the special day is tomorrow", "Tomorrow" );
      } else {
        datestr = KGlobal::locale()->formatDate( sD, KLocale::FancyLongDate );
      }
      // Print the date span for multiday, floating events, for the
      // first day of the event only.
      if ( (*addrIt).span > 1 ) {
        QString endstr =
          KGlobal::locale()->formatDate( sD.addDays( (*addrIt).span - 1 ) );
        datestr += " -\n " + endstr;
      }

      label = new QLabel( datestr, this );
      label->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
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
        label->setText( i18np( "in 1 day", "in %1 days", (*addrIt).daysTo ) );
      }

      label->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
      mLayout->addWidget( label, counter, 2 );
      mLabels.append( label );

      // What
      QString what;
      switch( (*addrIt).category ) {
      case CategoryBirthday:
        what = i18n( "Birthday" );
        break;
      case CategoryAnniversary:
        what = i18n( "Anniversary" );
        break;
      case CategoryHoliday:
        what = i18n( "Holiday" );
        break;
      case CategoryOther:
        what = i18n( "Special Occasion" );
        break;
      }
      label = new QLabel( this );
      label->setText( what );
      label->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
      mLayout->addWidget( label, counter, 3 );
      mLabels.append( label );

      // Description
      if ( (*addrIt).type == IncidenceTypeContact ) {
        KUrlLabel *urlLabel = new KUrlLabel( this );
        urlLabel->installEventFilter( this );
        urlLabel->setUrl( (*addrIt).item.url( Akonadi::Item::UrlWithMimeType ).url() );
        urlLabel->setText( (*addrIt).addressee.realName() );
        urlLabel->setTextFormat( Qt::RichText );
        urlLabel->setWordWrap( true );
        mLayout->addWidget( urlLabel, counter, 4 );
        mLabels.append( urlLabel );

        connect( urlLabel, SIGNAL(leftClickedUrl(const QString&)),
                 this, SLOT(mailContact(const QString&)) );
        connect( urlLabel, SIGNAL(rightClickedUrl(const QString&)),
                 this, SLOT(popupMenu(const QString&)) );
      } else {
        label = new QLabel( this );
        label->setText( (*addrIt).summary );
        label->setTextFormat( Qt::RichText );
        mLayout->addWidget( label, counter, 4 );
        mLabels.append( label );
        if ( !(*addrIt).desc.isEmpty() ) {
          label->setToolTip( (*addrIt).desc );
        }
      }

     // Age
      if ( (*addrIt).category == CategoryBirthday ||
           (*addrIt).category == CategoryAnniversary ) {
        label = new QLabel( this );
        if ( (*addrIt).yearsOld <= 0 ) {
          label->setText( "" );
        } else {
          label->setText( i18np( "one year", "%1 years", (*addrIt).yearsOld ) );
        }
        label->setAlignment( Qt::AlignLeft | Qt::AlignVCenter );
        mLayout->addWidget( label, counter, 5 );
        mLabels.append( label );
      }

      counter++;
    }
  } else {
    label = new QLabel(
        i18np( "No special dates within the next 1 day",
               "No special dates pending within the next %1 days",
               mDaysAhead ), this );
    label->setAlignment( Qt::AlignHCenter | Qt::AlignVCenter );
    mLayout->addWidget( label, 0, 0 );
    mLabels.append( label );
  }

  QList<QLabel*>::ConstIterator lit;
  for ( lit = mLabels.constBegin(); lit != mLabels.constEnd(); ++lit ) {
    (*lit)->show();
  }
  setUpdatesEnabled( true );
}

void SDSummaryWidget::mailContact( const QString &url )
{
  const Akonadi::Item item = Akonadi::Item::fromUrl( url );
  if ( !item.isValid() ) {
    kDebug()<< "Invalid item found";
    return;
  }

  Akonadi::ItemFetchJob *job = new Akonadi::ItemFetchJob( item, this );
  job->fetchScope().fetchFullPayload();
  if ( !job->exec() )
    return;

  if ( job->items().isEmpty() )
    return;

  const KABC::Addressee contact = job->items().first().payload<KABC::Addressee>();

  KToolInvocation::invokeMailer( contact.fullEmail(), QString() );
}

void SDSummaryWidget::viewContact( const QString &url )
{
  const Akonadi::Item item = Akonadi::Item::fromUrl( url );
  if ( !item.isValid() ) {
    kDebug()<< "Invalid item found";
    return;
  }

  Akonadi::ContactViewerDialog dlg( this );
  dlg.setContact( item );
  dlg.exec();
}

void SDSummaryWidget::popupMenu( const QString &url )
{
  KMenu popup( this );
  const QAction *sendMailAction = popup.addAction(
    KIconLoader::global()->loadIcon( "mail-message-new", KIconLoader::Small ),
    i18n( "Send &Mail" ) );
  const QAction *viewContactAction = popup.addAction(
    KIconLoader::global()->loadIcon( "view-pim-contacts", KIconLoader::Small ),
    i18n( "View &Contact" ) );

  const QAction *ret = popup.exec( QCursor::pos() );
  if ( ret == sendMailAction ) {
    mailContact( url );
  } else if ( ret == viewContactAction ) {
    viewContact( url );
  }
}

bool SDSummaryWidget::eventFilter( QObject *obj, QEvent *e )
{
  if ( obj->inherits( "KUrlLabel" ) ) {
    KUrlLabel* label = static_cast<KUrlLabel*>( obj );
    if ( e->type() == QEvent::Enter ) {
      emit message( i18n( "Mail to:\"%1\"", label->text() ) );
    }
    if ( e->type() == QEvent::Leave ) {
      emit message( QString::null );	//krazy:exclude=nullstrassign for old broken gcc
    }
  }

  return KontactInterface::Summary::eventFilter( obj, e );
}

void SDSummaryWidget::dateDiff( const QDate &date, int &days, int &years )
{
  QDate currentDate;
  QDate eventDate;

  if ( QDate::isLeapYear( date.year() ) && date.month() == 2 && date.day() == 29 ) {
    currentDate = QDate( date.year(), QDate::currentDate().month(), QDate::currentDate().day() );
    if ( !QDate::isLeapYear( QDate::currentDate().year() ) ) {
      eventDate = QDate( date.year(), date.month(), 28 ); // celebrate one day earlier ;)
    } else {
      eventDate = QDate( date.year(), date.month(), date.day() );
    }
  } else {
    currentDate = QDate( QDate::currentDate().year(),
                         QDate::currentDate().month(),
                         QDate::currentDate().day() );
    eventDate = QDate( QDate::currentDate().year(), date.month(), date.day() );
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

ResourceCalendar *SDSummaryWidget::usingBirthdayResource()
{
  ResourceCalendar *resource = 0;
  CalendarResourceManager *manager = mCalendar->resourceManager();
  if ( !manager->isEmpty() ) {
    CalendarResourceManager::Iterator it;
    for ( it = manager->begin(); it != manager->end(); ++it ) {
      if ( (*it)->type() == QLatin1String( "birthdays" ) ) {
        resource = (*it);
        break;
      }
    }
  }
  return resource;
}

bool SDSummaryWidget::check( ResourceCalendar *cal, const QDate &date,
                             const QString &summary )
{
  if ( !cal ) {
    return false;
  }

  Event::List events = cal->rawEventsForDate( date );
  Event::List::ConstIterator it;
  for ( it = events.constBegin(); it != events.constEnd(); ++it ) {
    if ( (*it)->summary() == summary ) {
      return true;
    }
  }
  return false;
}

QStringList SDSummaryWidget::configModules() const
{
  return QStringList( "kcmsdsummary.desktop" );
}

#include "sdsummarywidget.moc"
