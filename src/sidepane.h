/* This file is part of the KDE project
   Copyright (C) 2003 Daniel Molkentin <molkentin@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
 */
#ifndef KONTACT_SIDEPANE_H
#define KONTACT_SIDEPANE_H

#include "sidepanebase.h"

#include <qguardedptr.h>
#include <qptrlist.h>
#include <qpushbutton.h>
#include <qvaluelist.h>
#include <qvbox.h>

class QWidgetStack;
class QLabel;

namespace Kontact
{

  class Core;
  class Plugin;

  ///////////////////////////////////////////////////////////////////////
  // Helper classes
  ///////////////////////////////////////////////////////////////////////


  class PanelButton : public QPushButton
  {
    Q_OBJECT
    public:
      PanelButton(Kontact::Plugin *plugin, int id, QWidget *parent, const char* name = 0);

      ~PanelButton() {}

      bool isActive() const { return mActive; }

      void setActive();
      void setInactive();

      int id() const { return mId; }

      Kontact::Plugin *plugin() const { return mPlugin; }

    signals:
      void clicked( PanelButton *pb );
      void showPart( Kontact::Plugin* plugin );

    public slots:
      void slotClicked();

    protected:
      virtual void setPixmap(const QPixmap&);
      virtual void setText(const QString&);
      virtual void drawButtonLabel(QPainter *p);
      void composeLabel(QPainter *p);

    private:
      Kontact::Plugin *mPlugin;
      QPixmap mPix;
      QString mText;
      bool mActive;
      bool mId;
  };

  ///////////////////////////////////////////////////////////////////////

  class SidePane : public SidePaneBase
  {
    Q_OBJECT
    public:
      SidePane( Core* core, QWidget *parent, const char* name = 0 );
      ~SidePane();

    public slots:
      void switchItems(PanelButton* pb);

      void updatePlugins();
      void selectPlugin( Kontact::Plugin* );
      void selectPlugin( const QString &pluginName );

    protected slots:
      void switchSidePaneWidget( Kontact::Plugin * );

    private:
      QWidgetStack* mContentStack;
      QLabel* mHeaderWidget;
      QPtrList<PanelButton> mButtonList;
      QValueList<QGuardedPtr<QWidget> > mContentList;
  };

}

#endif
