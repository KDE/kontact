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

#include <dcopclient.h>
#include <dcopref.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>

#include "summarywidget.h"

SummaryWidget::SummaryWidget( QWidget *parent, const char *name )
  : QTextBrowser( parent, name ),
    DCOPObject( "WeatherSummaryWidget" )
{
  if ( !connectDCOPSignal( 0, 0, "fileUpdate(QString)", "refresh(QString)", false ) )
    kdDebug() << "Could not attach signal..." << endl;
  else
    kdDebug() << "attached dcop signals..." << endl;

  QString error;
  QCString appID;

  bool serviceAvailable = true;
  if ( !kapp->dcopClient()->isApplicationRegistered( "KWeatherService" ) ) {
    if ( !KApplication::startServiceByDesktopName( "kweatherservice", QStringList(), &error, &appID ) ) {
      setText( error );
      serviceAvailable = false;
    }
  }

  if ( serviceAvailable ) {
    DCOPRef dcopCall( "KWeatherService", "WeatherService" );
    DCOPReply reply = dcopCall.call( "listStations()", true );
    if ( reply.isValid() ) {
      mStations = reply;

      connect( &mTimer, SIGNAL( timeout() ), this, SLOT( timeout() ) );
      mTimer.start( 0 );
    } else {
      setText( "ERROR: dcop reply not valid" );
    }
  }
}

void SummaryWidget::updateView()
{
  clear();

  if ( mStations.count() == 0 ) {
    setText( i18n( "No weather stations defined." ) );
    return;
  }

  WeatherData data = mWeatherMap[ mStations[ 0 ] ];

  QString cover;
  for ( uint i = 0; i < data.cover().count(); ++i )
    cover += QString( "<li>%1</li>" ).arg( data.cover()[ i ] );

  QString line1 = QString( "<tr><td><img src=\"weather_icon\" width=\"64\" height=\"64\"></td><td valign=\"top\">%1</td><td></td><td></td></tr>" )
                         .arg( "<ul>" + cover + "</ul>" );
  QString line2 = QString( "<tr><td align=\"right\"><b>%1:</b></td><td>%2</td><td align=\"right\"><b>%3:</b></td><td>%4</td></tr>" )
                         .arg( i18n( "Temperature" ) )
                         .arg( data.temperature() )
                         .arg( i18n( "Dew Point" ) )
                         .arg( data.dewPoint() );

  QString line3 = QString( "<tr><td align=\"right\"><b>%1:</b></td><td>%2</td><td align=\"right\"><b>%3:</b></td><td>%4</td></tr>" )
                         .arg( i18n( "Air Pressure" ) )
                         .arg( data.pressure() )
                         .arg( i18n( "Rel. Humidity" ) )
                         .arg( data.relativeHumidity() );

  QString line4 = QString( "<tr><td align=\"right\"><b>%1:</b></td><td>%2</td><td align=\"right\"></td><td></td></tr>" )
                         .arg( i18n( "Wind Speed" ) )
                         .arg( data.windSpeed() );

  QMimeSourceFactory::defaultFactory()->setPixmap( "weather_icon", data.icon() );

  QString text = QString( "<html><body><h1>%1</h1><table>%2</table></body></html>" )
                        .arg( i18n( "Weather Report" ) )
                        .arg( line1 + line2 + line3 + line4 );
  setText( text );
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
  mWeatherMap[ station ].setCover( dcopCall.call( "cover(QString)", station, true ) );
  mWeatherMap[ station ].setTemperature( dcopCall.call( "temperature(QString)", station, true ) );
  mWeatherMap[ station ].setPressure( dcopCall.call( "pressure(QString)", station, true ) );
  mWeatherMap[ station ].setWindSpeed( dcopCall.call( "wind(QString)", station, true ) );
  mWeatherMap[ station ].setDewPoint( dcopCall.call( "dewPoint(QString)", station, true ) );
  mWeatherMap[ station ].setRelativeHumidity( dcopCall.call( "relativeHumidity(QString)", station, true ) );

  updateView();
}

#include "summarywidget.moc"
