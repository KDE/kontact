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

#include <qimage.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qfile.h>
#include <qlabel.h>
#include <qtextedit.h>
#include <qvbox.h>

#include <dcopclient.h>
#include <dcopref.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kglobal.h>
#include <kglobalsettings.h>
#include <kiconloader.h>
#include <klocale.h>
#include <kurllabel.h>
#include <kdialogbase.h>
#include <kmessagebox.h>

#include "pilotDaemonDCOP_stub.h"

#include <ktextedit.h>

#include "summarywidget.h"

SummaryWidget::SummaryWidget( QWidget *parent, const char *name )
  : Kontact::Summary( parent, name ),
    DCOPObject( "KPilotSummaryWidget" ),
    mDCOPSuccess(false),
    mStartedDaemon(false),
    mShouldStopDaemon(true)
{
  mLayout = new QGridLayout( this, 2, 3, 3, 3 );

  int row=0;
  QPixmap icon = KGlobal::iconLoader()->loadIcon( "kpilot", KIcon::Desktop, KIcon::SizeMedium );
  QWidget *header = createHeader( this, icon, i18n( "KPilot Information" ) );
  mLayout->addMultiCellWidget( header, row,row, 0,3 );

  // Last sync information
  row++;
  mSyncTimeTextLabel = new QLabel( i18n("<i>Last sync:</i>"), this);
  mLayout->addWidget( mSyncTimeTextLabel, row, 0 );
  mSyncTimeLabel = new QLabel( i18n("No information available" ), this );
  mLayout->addWidget( mSyncTimeLabel, row, 1 );
  mShowSyncLogLabel = new KURLLabel( "", i18n("[View Sync Log]"), this );
  mLayout->addWidget( mShowSyncLogLabel, row, 3 );
  connect( mShowSyncLogLabel, SIGNAL( leftClickedURL( const QString& ) ),
    this, SLOT( showSyncLog( const QString& ) ) );

  // User
  row++;
  mPilotUserTextLabel = new QLabel( i18n("<i>User:</i>"), this);
  mLayout->addWidget( mPilotUserTextLabel, row, 0);
  mPilotUserLabel = new QLabel( i18n("Unknown"), this );
  mLayout->addMultiCellWidget( mPilotUserLabel, row, row, 1,3 );

  // Device information
  row++;
  mPilotDeviceTextLabel = new QLabel( i18n("<i>Device:</i>"), this);
  mLayout->addWidget( mPilotDeviceTextLabel, row, 0 );
  mPilotDeviceLabel = new QLabel( i18n("Unknown"), this );
  mLayout->addMultiCellWidget( mPilotDeviceLabel, row, row, 1,3 );

  // Status
  row++;
  mDaemonStatusTextLabel = new QLabel( i18n("<i>Status:</i>"), this);
  mLayout->addWidget( mDaemonStatusTextLabel, row, 0);
  mDaemonStatusLabel = new QLabel( i18n("No communication with the daemon possible"), this );
  mLayout->addMultiCellWidget( mDaemonStatusLabel, row, row, 1,3 );

  // Conduits:
  row++;
  mConduitsTextLabel = new QLabel( i18n("<i>Conduits:</i>"), this );
  mConduitsTextLabel->setAlignment(AlignAuto | AlignTop | ExpandTabs);
  mLayout->addWidget( mConduitsTextLabel, row, 0 );
  mConduitsLabel = new QLabel( i18n("No information available"), this );
  mConduitsLabel->setAlignment( mConduitsLabel->alignment()|Qt::WordBreak );
  mLayout->addMultiCellWidget( mConduitsLabel, row,row, 1,3 );
  
  // widgets shown if kpilotDaemon is not running
  row++;
  mNoConnectionLabel = new QLabel( i18n("KPilot is currently not running."), this );
  mLayout->addMultiCellWidget( mNoConnectionLabel, row, row, 1, 2 );
  mNoConnectionStartLabel = new KURLLabel( "", i18n("[Start KPilot]"), this );
  mLayout->addWidget( mNoConnectionStartLabel, row, 3 );
  connect( mNoConnectionStartLabel, SIGNAL( leftClickedURL( const QString& ) ),
    this, SLOT( startKPilot() ) );

//  mLayout->addStretch( 1 );
//  mLayout->addWidget( new QSpacerItem( 1, 20, QSizePolicy::Minimum, QSizePolicy::Expanding ) );

  if ( !kapp->dcopClient()->isApplicationRegistered( "kpilotDaemon" ) ) {
    startKPilot();
  }

  connectDCOPSignal( 0, 0, "kpilotDaemonStatusChanged()", "refresh()", false );
  refresh();
}

SummaryWidget::~SummaryWidget()
{
  if (mStartedDaemon && mShouldStopDaemon)
  {
    PilotDaemonDCOP_stub dcopToDaemon( "kpilotDaemon", "KPilotDaemonIface" );
    dcopToDaemon.quitNow(); // ASYNC, always succeeds.
  }
}

QStringList SummaryWidget::configModules() const
{
  QStringList modules;
  modules << "kpilot_config.desktop";
  return modules;
}

void SummaryWidget::refresh( )
{
  PilotDaemonDCOP_stub dcopToDaemon( "kpilotDaemon", "KPilotDaemonIface" );
  mDCOPSuccess = true;

  mLastSyncTime = dcopToDaemon.lastSyncDate();
  // check if that dcop call was successful
  mDCOPSuccess = mDCOPSuccess && dcopToDaemon.ok();

  mDaemonStatus = dcopToDaemon.shortStatusString();
  mDCOPSuccess = mDCOPSuccess && dcopToDaemon.ok();

  mConduits = dcopToDaemon.configuredConduitList();
  mDCOPSuccess = mDCOPSuccess && dcopToDaemon.ok();

  mSyncLog = dcopToDaemon.logFileName();
  mDCOPSuccess = mDCOPSuccess && dcopToDaemon.ok();

  mUserName = dcopToDaemon.userName();
  mDCOPSuccess = mDCOPSuccess && dcopToDaemon.ok();

  mPilotDevice = dcopToDaemon.pilotDevice();
  mDCOPSuccess = mDCOPSuccess && dcopToDaemon.ok();

  mShouldStopDaemon = dcopToDaemon.killDaemonOnExit();
  mDCOPSuccess = mDCOPSuccess && dcopToDaemon.ok();

  updateView();
}


void SummaryWidget::updateView()
{
  if (mDCOPSuccess)
  {
    if ( mLastSyncTime.isValid() ) {
      mSyncTimeLabel->setText( mLastSyncTime.toString(Qt::LocalDate) );
    } else {
      mSyncTimeLabel->setText( i18n("No information available") );
    }
    if (!mSyncLog.isEmpty()) {
      mShowSyncLogLabel->setEnabled(true);
      mShowSyncLogLabel->setURL( mSyncLog );
    } else {
      mShowSyncLogLabel->setEnabled(false);
    }
    mPilotUserLabel->setText( (mUserName.isEmpty())?i18n("unknown"):mUserName );
    mPilotDeviceLabel->setText( (mPilotDevice.isEmpty())?i18n("unknown"):mPilotDevice );
    mDaemonStatusLabel->setText( mDaemonStatus );
    mConduitsLabel->setText( mConduits.join(", ") );
  }
  else
  {
    mSyncTimeLabel->setText( i18n("No information available (Daemon not running?)" ) );
    mShowSyncLogLabel->setEnabled(false);
    mPilotUserLabel->setText( i18n("unknown" ) );
    mPilotDeviceLabel->setText( i18n("unknown" ) );
    mDaemonStatusLabel->setText( i18n("No communication with the daemon possible") );
    mConduitsLabel->setText( i18n("No information available") );
  }
  
  mSyncTimeTextLabel->setShown( mDCOPSuccess );
  mSyncTimeLabel->setShown( mDCOPSuccess );
  mShowSyncLogLabel->setShown( mDCOPSuccess );
  mPilotUserTextLabel->setShown( mDCOPSuccess );
  mPilotUserLabel->setShown( mDCOPSuccess );
  mPilotDeviceTextLabel->setShown( mDCOPSuccess );
  mPilotDeviceLabel->setShown( mDCOPSuccess );
  mDaemonStatusTextLabel->setShown( mDCOPSuccess );
  mDaemonStatusLabel->setShown( mDCOPSuccess );
  mConduitsTextLabel->setShown( mDCOPSuccess );
  mConduitsLabel->setShown( mDCOPSuccess );
  mNoConnectionLabel->setShown( !mDCOPSuccess );
  mNoConnectionStartLabel->setShown( !mDCOPSuccess );
}

void SummaryWidget::showSyncLog( const QString &filename )
{
	KDialogBase dlg( this, 0, true, QString::null, KDialogBase::Ok, KDialogBase::Ok );
	dlg.setCaption( i18n("KPilot HotSync Log") );
	QTextEdit *edit = new QTextEdit( dlg.makeVBoxMainWidget() );
	edit->setReadOnly(TRUE);


	QFile f(filename);
	if (!f.open(IO_ReadOnly))
	{
		KMessageBox::error( this, i18n("Unable to open Hotsync log %1.").arg(filename) );
		return;
	}

	QTextStream s(&f);
	while (!s.eof()) edit->append(s.readLine());

	edit->moveCursor(QTextEdit::MoveHome, false);

	f.close();

	dlg.setInitialSize( QSize( 400, 350) );
	dlg.exec();
}

void SummaryWidget::startKPilot()
{
  QString error;
  QCString appID;
//  if ( !KApplication::startServiceByDesktopName( "kpilotDaemon", QString( "--fail-silently" ),
//                                                 &error, &appID ) ) {
  if ( !KApplication::kdeinitExec( "kpilotDaemon", QString( "--fail-silently" ) ) ) {
    kdDebug(5602) << "No service available..." << endl;
    mStartedDaemon = true;
  }
}
#include "summarywidget.moc"
