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

//#include "pilotDaemonDCOP_stub.h"
#include "summarywidget.h"

SummaryWidget::SummaryWidget( QWidget *parent, const char *name )
  : Kontact::Summary( parent, name ),
    DCOPObject( "KPilotSummaryWidget" ),
    mDCOPSuccess(false)
{
  mLayout = new QVBoxLayout( this );

  QPixmap icon = KGlobal::iconLoader()->loadIcon( "kpilot", KIcon::Desktop, KIcon::SizeMedium );
  QWidget *header = createHeader( this, icon, i18n( "KPilot Information" ) );
  mLayout->addWidget( header );

  mSyncTimeLabel = new QLabel( i18n("No information about the last sync available" ), this );
  mLayout->addWidget( mSyncTimeLabel );
  mShowSyncLogLabel = new KURLLabel( "", "Show last sync log", this );
  mLayout->addWidget( mShowSyncLogLabel );
  connect( mShowSyncLogLabel, SIGNAL( leftClickedURL( const QString& ) ),
    this, SLOT( showSyncLog( const QString& ) ) );
  
  mPilotDeviceLabel = new QLabel( i18n("Hardware device for sync: Unknown"), this );
  mLayout->addWidget( mPilotDeviceLabel );
  mDaemonStatusLabel = new QLabel( i18n("No communication with the daemon possible"), this );
  mLayout->addWidget( mDaemonStatusLabel );
  mConduitsLabel = new QLabel( i18n("<i>Enabled conduits:</i>"), this );
  mLayout->addWidget( mConduitsLabel );
  mConduitsLabel = new QLabel( i18n("No information available"), this );
  mLayout->addWidget( mConduitsLabel );
  
  mLayout->addStretch( 1 );
//  mLayout->addWidget( new QSpacerItem( 1, 20, QSizePolicy::Minimum, QSizePolicy::Expanding ) );

  QString error;
  QCString appID;

  if ( !kapp->dcopClient()->isApplicationRegistered( "kpilotDaemon" ) ) {
    if ( !KApplication::startServiceByDesktopName( "kpilotDaemon", QStringList(), &error, &appID ) ) {
      kdDebug() << "No service available..." << endl;
    }
  }

  connectDCOPSignal( 0, 0, "kpilotDaemonStatusChanged()", "refresh()", false );
  refresh();
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
  
  mDaemonStatus = dcopToDaemon.statusString();
  mDCOPSuccess = mDCOPSuccess && dcopToDaemon.ok();
  
  mConduits = dcopToDaemon.configuredConduitList();
  mDCOPSuccess = mDCOPSuccess && dcopToDaemon.ok();

  mSyncLog = dcopToDaemon.logFileName();
  mDCOPSuccess = mDCOPSuccess && dcopToDaemon.ok();
  
  mUserName = dcopToDaemon.userName();
  mDCOPSuccess = mDCOPSuccess && dcopToDaemon.ok();
  
  mPilotDevice = dcopToDaemon.pilotDevice();
  mDCOPSuccess = mDCOPSuccess && dcopToDaemon.ok();
  
  updateView();
}


void SummaryWidget::updateView()
{
  if (mDCOPSuccess) 
  {
    if ( mLastSyncTime.isValid() ) {
      mSyncTimeLabel->setText( i18n("Last synced at %1 (user: %2)").arg( mLastSyncTime.toString() ).arg( mUserName ) );
    } else {
      mSyncTimeLabel->setText( i18n("Last sync time not available") );
    }
    mShowSyncLogLabel->setEnabled(true);
    mShowSyncLogLabel->setURL( mSyncLog );
    mPilotDeviceLabel->setText( i18n("Daemon listening on device %1").arg( mPilotDevice ) );
    mDaemonStatusLabel->setText( mDaemonStatus );
    mConduitsLabel->setText( mConduits.join(", ") );
  }
  else
  {
    mSyncTimeLabel->setText( i18n("No information about the last sync (Daemon not running?)" ) );
    mShowSyncLogLabel->setEnabled(false);
    mPilotDeviceLabel->setText( i18n("Daemon listening on device %1").arg( i18n("\"unknown\"") ) );
    mDaemonStatusLabel->setText( i18n("No communication with the daemon possible") );
    mConduitsLabel->setText( i18n("No information available") );
  }
}

void SummaryWidget::showSyncLog( const QString &filename )
{
	KDialogBase dlg( this, 0, true, QString::null, KDialogBase::Ok, KDialogBase::Ok );
	dlg.setCaption( i18n("KPilot HotSync log") );
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
	
//	dlg.adjustSize();
//	dlg.resize(dlg.size());
	dlg.setInitialSize( QSize( 400, 350) );


//	if ( width > 0 && height > 0 )
//		dlg.setInitialSize( QSize( width, height ) );

	dlg.exec();
}
#include "summarywidget.moc"
