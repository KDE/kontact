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

#include <qptrlist.h>
#include <qwidgetstack.h>
#include <qsignal.h>

#include <klocale.h>
#include <kiconloader.h>

#include <kdebug.h>

#include "sidepane.h"

using namespace Kontact;

///////////////////////////////////////////////////////////////////////
// Helper classes
///////////////////////////////////////////////////////////////////////

PanelButton::PanelButton(const QIconSet & icon, const QString & text, 
    int id, QObject* receiver, const char* slot, 
    QWidget *parent, const char* name) :
QPushButton(icon, text, parent, name)
{
  m_active = false;
  m_id = id;
  m_receiver = receiver;
  m_slot = slot;


  setFlat(true);
  connect(this, SIGNAL(clicked()), SLOT(slotClicked()));
}

void PanelButton::connectToReceiver(QObject* sender) {
  QSignal sig(sender);
  sig.connect(m_receiver, m_slot);
  sig.activate();
  kdDebug() << "PanelButton::connectToReceiver()" << endl;
}


void PanelButton::slotClicked()
{
  emit clicked(this, m_id);
  setActive();
}

void PanelButton::setActive()
{
  QColorGroup cg(colorGroup());
  cg.setColor(QColorGroup::Button, cg.highlight());
  cg.setColor(QColorGroup::ButtonText, cg.highlightedText());

  QPalette pal = palette();
  pal.setActive(cg);
  setPalette(pal);

  m_active = true;

  kdDebug() << "PanelButton::setActive()" << endl;
}

void PanelButton::setInactive()
{
  // reset using parents gc
  QColorGroup cg(parentWidget()->colorGroup());
  cg.setColor(QColorGroup::Button, cg.button());
  cg.setColor(QColorGroup::ButtonText, cg.buttonText());

  QPalette pal = palette();
  pal.setActive(cg);
  setPalette(pal);

  m_active = false;
}

///////////////////////////////////////////////////////////////////////

  SidePane::SidePane(QWidget *parent, const char* name)
: QVBox(parent, name), m_contentStack(0), m_headerWidget(0)
{

  setSpacing(0);
  m_headerWidget = new QPushButton(this, "header");
  m_headerWidget->setFlat(true);
  m_contentStack = new QWidgetStack(this);
}


void SidePane::addServiceEntry(const QPixmap& icon, const QString& text, 
    QObject* receiver, const char* slot) 
{
  //int id = m_contentStack->addWidget(child);
  PanelButton* pb = new PanelButton(icon, text, 0, receiver, slot, this, "PanelButton");
  m_buttonList.append(pb);
  connect(pb, SIGNAL(clicked(PanelButton*, int)), this, SLOT(switchItems(PanelButton*, int)));
}

void SidePane::switchItems(PanelButton* pb, int id)
{
  QPtrListIterator<PanelButton> it(m_buttonList);
  for (; it.current(); ++it)
  {
    if (it.current()->isActive())	
      it.current()->setInactive();
  }

  pb->connectToReceiver(this);
  m_contentStack->raiseWidget(id);
  m_headerWidget->setText(pb->text());
}

void SidePane::invokeFirstEntry()
{
  PanelButton *btn = m_buttonList.first();
  // no plugins loaded. Something is really broken..
  Q_ASSERT(btn);
  btn->slotClicked();
}

#include "sidepane.moc"

// vim: ts=2 sw=2 et
