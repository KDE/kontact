/*
    This file is part of Kaplan
    Copyright (c) 2001 Matthias Hoelzer-Kluepfel <mhk@kde.org>
    Copyright (c) 2002-2003 Daniel Molkentin <molkentin@kde.org>

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
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/


#ifndef __CORE_H__
#define __CORE_H__


#include <qwidgetstack.h>
#include <qptrlist.h>


#include <kparts/mainwindow.h>
#include <kparts/part.h>
#include <kparts/partmanager.h>


#include "kpcore.h"
#include <kdcopservicestarter.h>


namespace Kaplan
{
  class Plugin;
};

class SidePane;
class KAction;

class Core : public Kaplan::Core, public KDCOPServiceStarter
{
  Q_OBJECT

public:

  Core();
  ~Core();

  virtual void addMainEntry(QString text, QString icon, QObject *receiver, const char *slot);

  virtual void addPart(KParts::Part *part);

  virtual void showPart(KParts::Part *part);

  virtual void insertNewAction(KAction *action);

  // KDCOPServiceStarter interface
  virtual int startServiceFor( const QString& serviceType,
                               const QString& constraint = QString::null,
                               const QString& preferences = QString::null,
                               QString *error=0, QCString* dcopService=0,
                               int flags=0 );

private slots:

  void slotQuit();

  void activePartChanged(KParts::Part *part);

  void loadSettings();
  void saveSettings();

  void slotPreferences();
private:

  void loadPlugins();
  void addPlugin(Kaplan::Plugin *plugin);

  void setupActions();

  SidePane *m_sidePane;
      
  KParts::PartManager *m_partManager;

  QWidgetStack *m_stack;
  QPtrList<Kaplan::Plugin> m_plugins;

  KActionMenu *m_newActions;

};

#endif

// vim: ts=4 sw=4 et
