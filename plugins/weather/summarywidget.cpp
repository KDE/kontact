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

#include "summarywidget.h"

SummaryWidget::SummaryWidget( QWidget *parent, const char *name )
  : Kontact::Summary( parent, name ),
    DCOPObject( "WeatherSummaryWidget" )
{
  mLayout = new QVBoxLayout( this );

  QPixmap icon = KGlobal::iconLoader()->loadIcon( "kweather", KIcon::Desktop, KIcon::SizeMedium );
  QWidget *header = createHeader( this, icon, i18n( "Weather Information" ) );
  mLayout->addWidget( header );

  connectDCOPSignal( 0, 0, "fileUpdate(QString)", "refresh(QString)", false );
  connectDCOPSignal( 0, 0, "stationRemoved(QString)", "stationRemoved(QString)", false );

  QString error;
  QCString appID;

  bool serviceAvailable = true;
  if ( !kapp->dcopClient()->isApplicationRegistered( "KWeatherService" ) ) {
    if ( !KApplication::startServiceByDesktopName( "kweatherservice", QStringList(), &error, &appID ) ) {
      kdDebug() << "No service available..." << endl;
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
      kdDebug() << "ERROR: dcop reply not valid..." << endl;
    }
  }
}

int SummaryWidget::summaryHeight() const
{
  if ( mStations.isEmpty() )
    return 0;
  else
    return 1;
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
    kdDebug() << "No weather stations defined..." << endl;
    return;
  }

  int counter = 0;
  QMap<QString, WeatherData>::Iterator it;
  for ( it = mWeatherMap.begin(); it != mWeatherMap.end() && counter < 3; ++it ) {
    WeatherData data = it.data();

    QString cover;
    for ( uint i = 0; i < data.cover().count(); ++i )
      cover += QString( "- %1\n" ).arg( data.cover()[ i ] );

    QImage img;
    img = data.icon();

    QGridLayout *layout = new QGridLayout( mLayout, 3, 3, 3 );
    mLayout->addStretch( 10 );
    mLayouts.append( layout );

    QLabel *label = new QLabel( this );
    label->setPixmap( img.smoothScale( 32, 32 ) );
    label->setMaximumSize(label->sizeHint());
    label->setAlignment(/* AlignRight |*/ AlignTop );
    layout->addMultiCellWidget( label, 0, 1, 0, 0 );
    mLabels.append( label );

    label = new QLabel( this );
    label->setText( QString( "%1 (%2)" ).arg( data.name() ).arg( data.temperature() ) );
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
                         .arg( data.windSpeed() )
                         .arg( i18n( "Rel. Humidity" ) )
                         .arg( data.relativeHumidity() );

    QToolTip::add( label, labelText.replace( " ", "&nbsp;" ) );

    label = new QLabel( cover, this );
    label->setAlignment( AlignLeft );
    layout->addMultiCellWidget( label, 1, 1, 1, 2 );
    mLabels.append( label );

    counter++;
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

  updateView();
}

void SummaryWidget::stationRemoved( QString station )
{
  mWeatherMap.remove( station );
  updateView();
}

#include "summarywidget.moc"
