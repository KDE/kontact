#ifndef PILOTDAEMONDCOP_H
#define PILOTDAEMONDCOP_H
/* pilotDaemonDCOP.h			KPilotDaemon
**
** Copyright (C) 2000 by Adriaan de Groot
**
** This file defines the DCOP interface for
** the KPilotDaemon. The daemon has *two* interfaces:
** one belonging with KUniqueApplication and this one.
*/

/*
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program in a file called COPYING; if not, write to
** the Free Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
** MA 02111-1307, USA.
*/

/*
** Bug reports and questions can be sent to kde-pim@kde.org
*/


#ifndef _DCOPOBJECT_H
#include <dcopobject.h>
#endif

#include <qdatetime.h>
#include <qstringlist.h>

class PilotDaemonDCOP : virtual public DCOPObject
{
	K_DCOP
public:
	/**
	* You can pass values of this type to requestSync; since
	* you pass an int, all the values are explicitly defined here.
	*/
	enum HotSyncTypes { Test=0,
		HotSync=1,
		FastSync=2,
		Backup=3,
		Restore=4 } ;
k_dcop:
	/**
	* Start a HotSync. What kind of HotSync is determined
	* by the int parameter (use the enum above!):
	*/
	virtual ASYNC requestSync(int) = 0;
	virtual ASYNC requestFastSyncNext() = 0;
	virtual ASYNC requestRegularSyncNext() = 0;
	virtual int nextSyncType() const = 0;

	/**
	* Functions for the KPilot UI, indicating what the daemon
	* should do.
	*/
	virtual ASYNC quitNow() = 0;
	virtual ASYNC reloadSettings() =0; // Indicate changed config file.

	/**
	* Functions requesting the status of the daemon.
	*/
	virtual QString statusString() = 0;
	
	/** 
	* Functions reporting same status data, e.g. for the kontact plugin.
	*/
	virtual QDateTime lastSyncDate() = 0;
	virtual QStringList configuredConduitList() = 0;
	virtual QString logFileName() = 0;
	virtual QString userName() = 0;
	virtual QString pilotDevice() = 0;
k_dcop_signals:
	void kpilotDaemonStatusChanged();
} ;

#endif
