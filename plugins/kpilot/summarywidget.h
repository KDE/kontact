/*
    This file is part of Kontact.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>
    Copyright (C) 2004 Reinhold Kainhofer <reinhold@kainhofer.com>

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

#include "summary.h"

#include <dcopobject.h>
#include <pilotDaemonDCOP.h>

#include <qmap.h>
#include <qpixmap.h>
#include <qptrlist.h>
#include <qstringlist.h>
#include <qtimer.h>
#include <qwidget.h>
#include <qdatetime.h>

class QGridLayout;
class QLabel;
class KURLLabel;

class SummaryWidget : public Kontact::Summary, public DCOPObject
{
    Q_OBJECT
    K_DCOP
  public:
    SummaryWidget( QWidget *parent, const char *name = 0 );

    int summaryHeight() const { return 1; }

    QStringList configModules() const;

  k_dcop:
    void refresh( );

  private slots:
    void updateView();
    void showSyncLog( const QString &filename );

  private:
    QTimer mTimer;
    
    QLabel*mSyncTimeLabel;
    KURLLabel*mShowSyncLogLabel;
    QLabel*mPilotUserLabel;
    QLabel*mPilotDeviceLabel;
    QLabel*mDaemonStatusLabel;
    QLabel*mConduitsLabel;
    
    QGridLayout *mLayout;
    
    QDateTime mLastSyncTime;
    QString mDaemonStatus;
    QStringList mConduits;
    QString mSyncLog;
    QString mUserName;
    QString mPilotDevice;
    bool mDCOPSuccess;
};

#endif
