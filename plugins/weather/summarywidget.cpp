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
#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtooltip.h>

#include <dcopclient.h>
#include <dcopref.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kurllabel.h>

#include "summarywidget.h"

SummaryWidget::SummaryWidget( QWidget *parent, const char *name )
  : Kontact::Summary( parent, name ),
    DCOPObject( "WeatherSummaryWidget" ), mProc( 0 )
{
  mLayout = new QVBoxLayout( this, 3, 3 );
  mLayout->setAlignment( Qt::AlignTop );

  QPixmap icon = KGlobal::iconLoader()->loadIcon( "kweather", KIcon::Desktop, KIcon::SizeMedium );
  QWidget *header = createHeader( this, icon, i18n( "Weather Information" ) );
  mLayout->addWidget( header );

  QString error;
  QCString appID;
  bool serviceAvailable = true;
  if ( !kapp->dcopClient()->isApplicationRegistered( "KWeatherService" ) ) {
    if ( KApplication::startServiceByDesktopName( "kweatherservice", QStringList(), &error, &appID ) ) {
      QLabel *label = new QLabel( i18n( "No weather dcop service available;\nyou need KWeather to use this plugin." ), this );
      mLayout->addWidget( label, Qt::AlignHCenter );
      serviceAvailable = false;
    }
  }

  if ( serviceAvailable ) {
    connectDCOPSignal( 0, 0, "fileUpdate(QString)", "refresh(QString)", false );
    connectDCOPSignal( 0, 0, "stationRemoved(QString)", "stationRemoved(QString)", false );

    DCOPRef dcopCall( "KWeatherService", "WeatherService" );
    DCOPReply reply = dcopCall.call( "listStations()", true );
    if ( reply.isValid() ) {
      mStations = reply;

      connect( &mTimer, SIGNAL( timeout() ), this, SLOT( timeout() ) );
      mTimer.start( 0 );
    } else {
      kdDebug(5602) << "ERROR: dcop reply not valid..." << endl;
    }
  }
}


void SummaryWidget::updateView()
{
  mLayouts.setAutoDelete( true );
  mLayouts.clear();
  mLayouts.setAutoDelete( false );

  mLabels.setAutoDelete( true );
  mLabels.clear();
  mLabels.setAutoDelete( false );

  if ( mStations.count() == 0 ) {
    kdDebug(5602) << "No weather stations defined..." << endl;
    return;
  }


  QValueList<WeatherData> dataList = mWeatherMap.values();
  qHeapSort( dataList );

  QValueList<WeatherData>::Iterator it;
  for ( it = dataList.begin(); it != dataList.end(); ++it ) {
    QString cover;
    for ( uint i = 0; i < (*it).cover().count(); ++i )
      cover += QString( "- %1\n" ).arg( (*it).cover()[ i ] );

    QImage img;
    img = (*it).icon();

    QGridLayout *layout = new QGridLayout( mLayout, 3, 3, 3 );
    mLayouts.append( layout );

    KURLLabel* urlLabel = new KURLLabel(this);
    urlLabel->installEventFilter(this);
    urlLabel->setURL((*it).stationID());
    urlLabel->setPixmap( img.smoothScale( 32, 32 ) );
    urlLabel->setMaximumSize(urlLabel->sizeHint());
    urlLabel->setAlignment(/* AlignRight |*/ AlignTop );
    layout->addMultiCellWidget( urlLabel, 0, 1, 0, 0 );
    mLabels.append( urlLabel );
    connect (urlLabel, SIGNAL(leftClickedURL( const QString&) ),
    	this, SLOT(slotShowReport(const QString& )));

    QLabel* label = new QLabel( this );
    label->setText( QString( "%1 (%2)" ).arg( (*it).name() ).arg( (*it).temperature() ) );
    QFont font = label->font();
    font.setBold( true );
    label->setFont( font );
    label->setAlignment( AlignLeft );
    layout->addMultiCellWidget( label, 0, 0, 1, 2 );
    mLabels.append( label );

    QString labelText;
    labelText = QString( "<b>%1:</b> %2<br>"
                         "<b>%3:</b> %4" )
                         .arg( i18n( "Wind Speed" ) )
                         .arg( (*it).windSpeed() )
                         .arg( i18n( "Rel. Humidity" ) )
                         .arg( (*it).relativeHumidity() );

    QToolTip::add( label, labelText.replace( " ", "&nbsp;" ) );

    label = new QLabel( cover, this );
    label->setAlignment( AlignLeft );
    layout->addMultiCellWidget( label, 1, 1, 1, 2 );
    mLabels.append( label );
  }

  for ( QLabel *label = mLabels.first(); label; label = mLabels.next() )
    label->show();

  mLayout->addStretch( 1 );
}

void SummaryWidget::timeout()
{
  mTimer.stop();

  DCOPRef dcopCall( "KWeatherService", "WeatherService" );
  dcopCall.send( "updateAll()" );

  mTimer.start( 15 * 60000 );
}

void SummaryWidget::refresh( QString station )
{
  DCOPRef dcopCall( "KWeatherService", "WeatherService" );

  mWeatherMap[ station ].setIcon( dcopCall.call( "currentIcon(QString)", station, true ) );
  mWeatherMap[ station ].setName( dcopCall.call( "stationName(QString)", station, true ) );
  mWeatherMap[ station ].setCover( dcopCall.call( "cover(QString)", station, true ) );
  mWeatherMap[ station ].setTemperature( dcopCall.call( "temperature(QString)", station, true ) );
  mWeatherMap[ station ].setWindSpeed( dcopCall.call( "wind(QString)", station, true ) );
  mWeatherMap[ station ].setRelativeHumidity( dcopCall.call( "relativeHumidity(QString)", station, true ) );
  mWeatherMap[ station ].setStationID(station);

  updateView();
}

void SummaryWidget::stationRemoved( QString station )
{
  mWeatherMap.remove( station );
  updateView();
}

QStringList SummaryWidget::configModules() const
{
  return QStringList( "kcmweatherservice.desktop" );
}

void SummaryWidget::slotShowReport(const QString &stationID)
{
  mProc = new KProcess;
  QApplication::connect(mProc, SIGNAL(processExited(KProcess *)),
	this, SLOT(slotReportFinished(KProcess* )));
  *mProc << "kweatherreport";
  *mProc << stationID;
  if ( !mProc->start() )
  {
    delete mProc;
    mProc=0;
  }
}

void SummaryWidget::slotReportFinished(KProcess* /*proc*/){
  delete mProc;
 mProc = 0;
}

#include "summarywidget.moc"
