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

#include <kmessagebox.h>
#include <kaction.h>
#include <kgenericfactory.h>
#include <kstatusbar.h>
#include <kparts/componentfactory.h>
#include <kdebug.h>

#include "kpcore.h"
#include "summaryview_plugin.h"
#include "summaryview_part.h"

#include "summaryview_plugin.moc"

typedef KGenericFactory< SummaryView, Kontact::Core > SummaryViewFactory;
K_EXPORT_COMPONENT_FACTORY( libkpsummaryplugin, SummaryViewFactory( "kpsummaryplugin" ) );

SummaryView::SummaryView(Kontact::Core *_core, const char *name, const QStringList &)
  : Kontact::Plugin(i18n("Summary"), "summary", _core, _core, name), m_part(0)
{
	setInstance(SummaryViewFactory::instance());

  //insertNewAction(new KAction("Summarry", 0, this, SLOT(slotTestMenu()), actionCollection(), "edit_test"));

   m_plugins = _core->pluginList();
   setXMLFile("kpsummaryplugin.rc");
}

SummaryView::~SummaryView()
{
}

KParts::Part* SummaryView::part()
{
	if (!m_part)
	{
		m_part = new SummaryViewPart(this, "summarypart", m_plugins );
	}

	return m_part;
}
