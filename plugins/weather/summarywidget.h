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

#ifndef SUMMARYWIDGET_H
#define SUMMARYWIDGET_H

#include "summary.h"

#include <dcopobject.h>

#include <qmap.h>
#include <qpixmap.h>
#include <qptrlist.h>
#include <qstringlist.h>
#include <qtimer.h>
#include <qwidget.h>

class KProcess;

class QGridLayout;
class QLabel;
class QVBoxLayout;

class WeatherData
{
  public:
    void setIcon( const QPixmap &icon ) { mIcon = icon; }
    QPixmap icon() const { return mIcon; }

    void setName( const QString &name ) { mName = name; }
    QString name() const { return mName; }

    void setCover( const QStringList& cover ) { mCover = cover; }
    QStringList cover() const { return mCover; }

    void setDate( const QString &date ) { mDate = date; }
    QString date() const { return mDate; }

    void setTemperature( const QString &temperature ) { mTemperature = temperature; }
    QString temperature() const { return mTemperature; }

    void setWindSpeed( const QString &windSpeed ) { mWindSpeed = windSpeed; }
    QString windSpeed() const { return mWindSpeed; }

    void setRelativeHumidity( const QString &relativeHumidity ) { mRelativeHumidity = relativeHumidity; }
    QString relativeHumidity() const { return mRelativeHumidity; }

    void setStationID( const QString &station ) { mStationID = station;}
    QString stationID() { return mStationID; }

    bool operator< ( const WeatherData &data )
    {
      return ( QString::localeAwareCompare( mName, data.mName ) < 0 );
    }

  private:
    QPixmap mIcon;
    QString mName;
    QStringList mCover;
    QString mDate;
    QString mTemperature;
    QString mWindSpeed;
    QString mRelativeHumidity;
    QString mStationID;
};

class SummaryWidget : public Kontact::Summary, public DCOPObject
{
    Q_OBJECT
    K_DCOP
  public:
    SummaryWidget( QWidget *parent, const char *name = 0 );

    QStringList configModules() const;

    void updateSummary( bool force = false );

  k_dcop:
    virtual void refresh( QString );
    virtual void stationRemoved( QString );

  protected:
    virtual bool eventFilter( QObject *obj, QEvent *e );

  private slots:
    void updateView();
    void timeout();
    void showReport( const QString& );
    void reportFinished( KProcess* );

  private:
    QStringList mStations;
    QMap<QString, WeatherData> mWeatherMap;
    QTimer mTimer;

    QPtrList<QLabel> mLabels;
    QPtrList<QGridLayout> mLayouts;
    QVBoxLayout *mLayout;

    KProcess* mProc;
};

#endif
