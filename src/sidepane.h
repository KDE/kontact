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
#ifndef  SIDEPANE_H
#define  SIDEPANE_H

#include <qpushbutton.h>
#include <qvbox.h>
#include <qptrlist.h>

class QWidgetStack;
class QLabel;

namespace Kontact
{

  class Plugin;

  ///////////////////////////////////////////////////////////////////////
  // Helper classes
  ///////////////////////////////////////////////////////////////////////


  class PanelButton : public QPushButton
  {
    Q_OBJECT
    public:
      PanelButton(Kontact::Plugin *plugin, int id, QWidget *parent, const char* name = 0);

      ~PanelButton() {};

      bool isActive() const { return m_active; };

      void setActive();
      void setInactive();

      int id() const { return m_id; }

      Kontact::Plugin *plugin() const { return m_plugin; }

    signals:
      void clicked(PanelButton* pb);
      void showPart(KParts::Part* part);

    public slots:
      void slotClicked();

    protected:
      virtual void setPixmap(const QPixmap&);
      virtual void setText(const QString&);
      virtual void drawButtonLabel(QPainter *p);
      void composeLabel(QPainter *p);

    private:
      Kontact::Plugin *m_plugin;
      QPixmap m_pix;
      QString m_text;
      bool m_active;
      bool m_id;
  };

  ///////////////////////////////////////////////////////////////////////

  class SidePane : public QVBox
  {
    Q_OBJECT
    public:
      SidePane(QWidget *parent, const char* name = 0);
      ~SidePane() {};

    signals:
      void showPart(KParts::Part*);

    public slots:
      /**
       * adds a new entry to the sidepane
       **/
      void addEntry(Kontact::Plugin *plugin);

      void switchItems(PanelButton* pb);

      void invokeFirstEntry();

    protected slots:
      void switchSidePaneWidget(KParts::Part* part);

    private:
      QWidgetStack* m_contentStack;
      QLabel* m_headerWidget;
      QPtrList<PanelButton> m_buttonList;
  };

};
#endif // SIDEPANE_H

// vim: ts=2 sw=2 et
