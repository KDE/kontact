/*
   This file is part of KDE Kontact.

   Copyright (c) 2001 Matthias Hoelzer-Kluepfel <mhk@kde.org>
   Copyright (c) 2002-2003 Daniel Molkentin <molkentin@kde.org>

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

#ifndef KONTACT_PLUGIN_H
#define KONTACT_PLUGIN_H

#include <qobject.h>
#include <qwidget.h>
#include <kxmlguiclient.h>
#include <qptrlist.h>

class QStringList;
class DCOPClient;
class DCOPObject;
class KAboutData;
class KAction;

namespace Kontact
{
  class Core;

  /**
   * Base class for all Plugins in Kontact. Inherit from it
   * to get a plugin. It can insert an icon into the sidepane,
   * add widgets to the widgetstack and add menu items via XMLGUI.
   */
  class Plugin : public QObject, virtual public KXMLGUIClient
  {
      Q_OBJECT
    public:
      /**
       * Creates a new Plugin, note that @param name is required if
       * you want your plugin to do dcop via it's own instance of
       * @ref DCOPClient by calling @ref dcopClient.
       */
      Plugin( const QString& pluginName, const QString& icon, Core *core,
              QObject *parent, const char *name );

      ~Plugin();

      /**
        Insert "New" action.
      */
      void insertNewAction( KAction *action );

      /**
        Return user visible translated name of plugin.
      */
      QString pluginName() const;

      /**
       * Returns the name of the icon
       */
      QString icon() const;

      /**
        Called when the KPart associated with plugin is to be shown. The
        default implementation calls Kontact::Core::showPart().
      */
      void showPart( Kontact::Plugin *plugin );


      /**
       * Create the DCOP interface for the given @p serviceType, if this
       * plugin provides it. Return false otherwise.
       */
      virtual bool createDCOPInterface( const QString& /*serviceType*/ )
      {
        return 0;
      }

      /**
       * Reimplement this method and return a @ref QStringList of all config
       * modules your application part should offer via Kontact. Note that the
       * part and the module will have to take care for config syncing themselves.
       * Usually @p DCOP is used for that purpose.
       *
       * @note Make sure you offer the modules in the form:
       * <code>"pathrelativetosettings/mysettings.desktop"</code>
       *
       **/
      virtual QStringList configModules() const { return QStringList(); }

      /**
        Return about data of the application associated with the plugin. The
        about data will be used for display in the common Kontact about
        dialog.
      */
      virtual KAboutData *aboutData() { return 0; }

      /**
        Return the KPart associated with the plugin.
      */
      virtual KParts::Part *part() = 0;

      /**
        Return widget representing the plugin in the Kontact summary view. The
        default implementation returns no widget.
      */
      virtual QWidget *createSummaryWidget( QWidget * ) { return 0; }

      /**
        Returns if the plugin should be shown in the sidebar. The default
        implementation returns true.
      */
      virtual bool showInSideBar() const { return true; }

      /**
       * Retrieve the current DCOP Client for the plugin.
       *
       * The clients name is taken from the name argument in the constructor.
       * @note: The DCOPClient object will only be created when this method is
       * called for the first time. Make sure that the part has been loaded
       * before calling this method, if it's the one that contains the DCOP
       * interface that other parts might use.
       **/
      DCOPClient *dcopClient() const;

      /**
        FIXME: write API doc for Kontact::Plugin::newActions().
      */
      QPtrList<KAction> *newActions() const; 

    signals:
      /**
       * Emitted when the part will be shown. If you really want to avoid that
       * the part is shown at all, you will have to reimplement showPart();
       **/
      void aboutToShowPart();

    protected:
      /**
        Provide access to the Kontact core.
      */
      Core *core() const;

    private:
      class Private;
      Private *d;
  };
}

#endif

// vim: ts=2 sw=2 et
