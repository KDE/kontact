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

class QWidgetStack;

template <class T> class QPtrList;

namespace Kontact
{

  ///////////////////////////////////////////////////////////////////////
  // Helper classes
  ///////////////////////////////////////////////////////////////////////


  class PanelButton : public QPushButton
  {
    Q_OBJECT
    public:
      PanelButton(const QIconSet & icon, const QString & text, 
          int id, QObject* receiver, const char* slot,
          QWidget *parent, const char* name = 0);

      ~PanelButton() {};

      bool isActive() const { return m_active; };

      /**
       * Does the virtal connection
       **/
      void connectToReceiver( QObject * );

      void setActive();
      void setInactive();

signals:
      void clicked(PanelButton* pb, int id);

      public slots:	
        void slotClicked();


    private:
      bool m_active;
      bool m_id;
      QObject* m_receiver;
      QCString m_slot;
  };

  ///////////////////////////////////////////////////////////////////////

  class SidePane : public QVBox
  {
    Q_OBJECT
    public:	
      SidePane(QWidget *parent, const char* name = 0);
      ~SidePane() {};

      public slots:
        /**
         * adds a new service to the sidepane
         **/
        void addServiceEntry(const QPixmap& icon, const QString& text, 
            QObject* receiver, const char* slot); 

      void switchItems(PanelButton* pb, int id);

      void invokeFirstEntry();

    private:
      QWidgetStack* m_contentStack;
      QPushButton* m_headerWidget;
      QPtrList<PanelButton> m_buttonList;
  };

};
#endif // SIDEPANE_H 

// vim: ts=2 sw=2 et
