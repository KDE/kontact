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

#ifndef SUMMARYWIDGET_H
#define SUMMARYWIDGET_H

#include <dcopobject.h>

#include <qmap.h>
#include <qpixmap.h>
#include <qtextbrowser.h>
#include <qtimer.h>

class WeatherData
{
  public:
    void setIcon( const QPixmap &icon ) { mIcon = icon; }
    QPixmap icon() const { return mIcon; }

    void setCover( const QStringList& cover ) { mCover = cover; }
    QStringList cover() const { return mCover; }

    void setTemperature( const QString &temperature ) { mTemperature = temperature; }
    QString temperature() const { return mTemperature; }

    void setPressure( const QString &pressure ) { mPressure = pressure; }
    QString pressure() const { return mPressure; }

    void setWindSpeed( const QString &windSpeed ) { mWindSpeed = windSpeed; }
    QString windSpeed() const { return mWindSpeed; }

    void setDewPoint( const QString &dewPoint ) { mDewPoint = dewPoint; }
    QString dewPoint() const { return mDewPoint; }

    void setRelativeHumidity( const QString &relativeHumidity ) { mRelativeHumidity = relativeHumidity; }
    QString relativeHumidity() const { return mRelativeHumidity; }

  private:
    QPixmap mIcon;
    QStringList mCover;
    QString mTemperature;
    QString mPressure;
    QString mWindSpeed;
    QString mDewPoint;
    QString mRelativeHumidity;
};

class SummaryWidget : public QTextBrowser, public DCOPObject
{
  Q_OBJECT
  K_DCOP

  public:
    SummaryWidget( QWidget *parent, const char *name = 0 );

  k_dcop:
    virtual void refresh( QString );

  private slots:
    void updateView();
    void timeout();

  private:
    QStringList mStations;
    QMap<QString, WeatherData> mWeatherMap;
    QTimer mTimer;
};

#endif
