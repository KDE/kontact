/* This file is part of the KDE project
   Copyright (C) 2003 Sven Lüppken <sven@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
 */

#ifndef __SUMMARYVIEW_PLUGIN_H__
#define __SUMMARYVIEW_PLUGIN_H__

#include "kpplugin.h"
#include <kparts/part.h>

class SummaryView : public Kontact::Plugin
{
  Q_OBJECT

public:

	SummaryView(Kontact::Core *core, const char *name, const QStringList & /*args*/);
	~SummaryView();

	virtual KParts::Part* part();
//  virtual bool createDCOPInterface( const QString& serviceType );

//protected slots:
//  void slotNewMail();

private:
	KParts::ReadOnlyPart *m_part;
	QPtrList<Kontact::Plugin> m_plugins;
};

#endif
