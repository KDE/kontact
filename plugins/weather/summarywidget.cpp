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
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/
#include <QImage>
#include <QLabel>
#include <QLayout>
#include <QToolTip>
//Added by qt3to4:
#include <QPixmap>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QEvent>
#include <QApplication>

#include <kdebug.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kprocess.h>
#include <kurllabel.h>
#include <q3tl.h>
#include <ktoolinvocation.h>
#include "serviceinterface.h"
#include "summarywidget.h"

SummaryWidget::SummaryWidget( QWidget *parent )
  : Kontact::Summary( parent ),
    /*DCOPObject( "WeatherSummaryWidget" ),*/ mProc( 0 )
{
  mLayout = new QVBoxLayout( this );
  mLayout->setSpacing( 3 );
  mLayout->setMargin( 3 );
  mLayout->setAlignment( Qt::AlignTop );

  QPixmap icon = KIconLoader::global()->loadIcon( "kweather", K3Icon::Desktop, K3Icon::SizeMedium );
  QWidget *header = createHeader( this, icon, i18n( "Weather Information" ) );
  mLayout->addWidget( header );

  QString error;
  QString appID;
  bool serviceAvailable = true;
  if(!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.KWeatherService")) {
    if ( KToolInvocation::startServiceByDesktopName( "kweatherservice", QStringList(), &error, &appID ) ) {
      QLabel *label = new QLabel( i18n( "No weather dbus service available;\nyou need KWeather to use this plugin." ), this );
      mLayout->addWidget( label, Qt::AlignHCenter | Qt::AlignVCenter );
      serviceAvailable = false;
    }
  }

  if ( serviceAvailable ) {
    QDBusConnection::sessionBus().connect(QString(), "/Service", "org.kde.kweather.service", "fileUpdate", this, SLOT(refresh(QString)));
    QDBusConnection::sessionBus().connect(QString(), "/Service", "org.kde.kweather.service", "stationRemoved", this, SLOT(stationRemoved(QString)));

    OrgKdeKweatherServiceInterface service( "org.kde.KWeatherService", "/Service", QDBusConnection::sessionBus() );
    QDBusReply<QStringList> reply = service.listStations();
    if ( reply.isValid() ) {
      mStations = reply;

      connect( &mTimer, SIGNAL( timeout() ), this, SLOT( timeout() ) );
      mTimer.start( 0 );
    } else {
      kDebug(5602) << "ERROR: D-Bus reply not valid..." << endl;
    }
  }
}


void SummaryWidget::updateView()
{
  qDeleteAll( mLayouts );
  mLayouts.clear();

  qDeleteAll( mLabels );
  mLabels.clear();

  if ( mStations.count() == 0 ) {
    kDebug(5602) << "No weather stations defined..." << endl;
    return;
  }


  QList<WeatherData> dataList = mWeatherMap.values();
  qSort( dataList );

  QList<WeatherData>::Iterator it;
  for ( it = dataList.begin(); it != dataList.end(); ++it ) {
    QString cover;
    for ( int i = 0; i < (*it).cover().count(); ++i )
      cover += QString( "- %1\n" ).arg( (*it).cover()[ i ] );

    QImage img = (*it).icon().toImage();

    QGridLayout *layout = new QGridLayout();
    mLayout->addItem( layout );
    layout->setSpacing( 3 );
    mLayouts.append( layout );

    KUrlLabel* urlLabel = new KUrlLabel( this );
    urlLabel->installEventFilter( this );
    urlLabel->setUrl( (*it).stationID() );
    urlLabel->setPixmap( QPixmap::fromImage( img.scaled( 32, 32, Qt::IgnoreAspectRatio, Qt::SmoothTransformation ) ) );
    urlLabel->setMaximumSize( urlLabel->sizeHint() );
    urlLabel->setAlignment( Qt::AlignTop );
    layout->addWidget( urlLabel, 0, 0, 2, 1);
    mLabels.append( urlLabel );
    connect ( urlLabel, SIGNAL( leftClickedUrl( const QString& ) ),
              this, SLOT( showReport( const QString& ) ) );

    QLabel* label = new QLabel( this );
    label->setText( QString( "%1 (%2)" ).arg( (*it).name() ).arg( (*it).temperature() ) );
    QFont font = label->font();
    font.setBold( true );
    label->setFont( font );
    label->setAlignment( Qt::AlignLeft );
    layout->addWidget( label, 0, 1, 1, 2 );
    mLabels.append( label );

    QString labelText;
    labelText = QString( "<b>%1:</b> %2<br>"
                         "<b>%3:</b> %4<br>"
                         "<b>%5:</b> %6" )
                         .arg( i18n( "Last updated on" ) )
                         .arg( (*it).date() )
                         .arg( i18n( "Wind Speed" ) )
                         .arg( (*it).windSpeed() )
                         .arg( i18n( "Rel. Humidity" ) )
                         .arg( (*it).relativeHumidity() );

    label->setToolTip( labelText.replace( " ", "&nbsp;" ) );

    label = new QLabel( cover, this );
    label->setAlignment( Qt::AlignLeft );
    layout->addWidget( label, 1, 1, 1, 2 );
    mLabels.append( label );
  }

  Q_FOREACH(QLabel *label, mLabels)
    label->show();
}

void SummaryWidget::timeout()
{
  mTimer.stop();

  OrgKdeKweatherServiceInterface service( "org.kde.KWeatherService", "/Service", QDBusConnection::sessionBus() );
  service.updateAll();

  mTimer.start( 15 * 60000 );
}

void SummaryWidget::refresh( QString station )
{
  OrgKdeKweatherServiceInterface service( "org.kde.KWeatherService", "/Service", QDBusConnection::sessionBus() );
  QByteArray iconB = service.currentIcon(station);
  QPixmap icon;
  icon.loadFromData(iconB);
  mWeatherMap[ station ].setIcon( icon );
  mWeatherMap[ station ].setName( service.stationName(station) );
  mWeatherMap[ station ].setCover( service.cover(station ) );
  mWeatherMap[ station ].setDate( service.date(station ) );
  mWeatherMap[ station ].setTemperature( service.temperature(station) );
  mWeatherMap[ station ].setWindSpeed( service.wind(station) );
  mWeatherMap[ station ].setRelativeHumidity( service.relativeHumidity(station) );
  mWeatherMap[ station ].setStationID(station);

  updateView();
}

void SummaryWidget::stationRemoved( QString station )
{
  mWeatherMap.remove( station );
  updateView();
}

bool SummaryWidget::eventFilter( QObject *obj, QEvent* e )
{
  if ( obj->inherits( "KUrlLabel" ) ) {
    if ( e->type() == QEvent::Enter )
      emit message(
        i18n( "View Weather Report for Station" ) );
    if ( e->type() == QEvent::Leave )
      emit message( QString::null );
  }

  return Kontact::Summary::eventFilter( obj, e );
}

QStringList SummaryWidget::configModules() const
{
  return QStringList( "kcmweatherservice.desktop" );
}

void SummaryWidget::updateSummary( bool )
{
  timeout();
}

void SummaryWidget::showReport( const QString &stationID )
{
  mProc = new KProcess;
  QApplication::connect( mProc, SIGNAL( processExited( KProcess* ) ),
                         this, SLOT( reportFinished( KProcess* ) ) );
  *mProc << "kweatherreport";
  *mProc << stationID;

  if ( !mProc->start() ) {
    delete mProc;
    mProc = 0;
  }
}

void SummaryWidget::reportFinished( KProcess* )
{
  mProc->deleteLater();
  mProc = 0;
}

#include "summarywidget.moc"
