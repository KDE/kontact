/*
    This file is part of Kaplan
    Copyright (c) 2001 Matthias Hoelzer-Kluepfel <mhk@kde.org>
    Copyright (c) 2002 Daniel Molkentin <molkentin@kde.org>

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


#include <kparts/mainwindow.h>
#include <kparts/part.h>
#include <kparts/partmanager.h>


#include "kpcore.h"


namespace Kaplan
{
  class Plugin;
};

class Navigator;
class KAction;

class Core : public Kaplan::Core
{
  Q_OBJECT

public:

  Core();
  ~Core();

  virtual void addMainEntry(QString text, QString icon, QObject *receiver, const char *slot);
  
  virtual void addPart(KParts::Part *part);

  virtual void showView(QWidget *view);

  virtual void insertNewAction(KAction *action);
  
private slots:

  void slotQuit();
  
  void activePartChanged(KParts::Part *part);

  void loadSettings();
  void saveSettings();
  
private:

  void loadPlugins();
  void addPlugin(Kaplan::Plugin *plugin);

  void setupActions();

  Navigator *m_navigator;

  KParts::PartManager *m_partManager;
  
  QWidgetStack *m_stack;

  KActionMenu *m_newActions;
  
};

#endif

// vim: ts=4 sw=4 et
