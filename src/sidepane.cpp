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
#include <qobjectlist.h>

#include <klocale.h>
#include <kiconloader.h>
#include <sidebarextension.h>

#include <kdebug.h>

#include "mainwindow.h"

#include "kpplugin.h"

#include "sidepane.h"

using namespace Kontact;

///////////////////////////////////////////////////////////////////////
// Helper classes
///////////////////////////////////////////////////////////////////////

PanelButton::PanelButton( Kontact::Plugin *plugin, int id,  QWidget *parent, const char* name) :
QPushButton(BarIcon(plugin->icon()), plugin->pluginName(), parent, name)
{
  m_active = false;
  m_id = id;
  m_plugin = plugin;

  setFlat(true);
  connect(this, SIGNAL(clicked()), SLOT(slotClicked()));
}

void PanelButton::slotClicked()
{
  emit clicked(this);

  KParts::Part* part = m_plugin->part();
  emit showPart(part);

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

  kdDebug(5600) << "PanelButton::setActive()" << endl;
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
  m_contentStack->addWidget(new QWidget(m_contentStack));
}


void SidePane::addEntry(Kontact::Plugin *plugin)
{
  //int id = m_contentStack->addWidget(child);
  PanelButton* pb = new PanelButton(plugin, 0, this, "PanelButton");
  m_buttonList.append(pb);
  connect(pb, SIGNAL(clicked(PanelButton*)), this, SLOT(switchItems(PanelButton*)));
  connect(pb, SIGNAL(showPart(KParts::Part*)), this, SIGNAL(showPart(KParts::Part*)));
  connect(pb, SIGNAL(showPart(KParts::Part*)), this, SLOT(switchSidePaneWidget(KParts::Part*)));
}

void SidePane::switchSidePaneWidget(KParts::Part* part)
{
 Q_ASSERT(part);

 QObjectList *l = part->queryList( "KParts::SideBarExtension" );
 KParts::SideBarExtension *sbe = static_cast<KParts::SideBarExtension*>(l->first());

 if (!sbe)
 {
   m_contentStack->raiseWidget(0);
   return;
 }

 if (m_contentStack->id(sbe->widget()) == -1)
   m_contentStack->addWidget(sbe->widget());

 m_contentStack->raiseWidget(sbe->widget());
}

void SidePane::switchItems(PanelButton* pb)
{
  QPtrListIterator<PanelButton> it(m_buttonList);
  for (; it.current(); ++it)
  {
    if (it.current()->isActive())
      it.current()->setInactive();
  }

  m_contentStack->raiseWidget(pb->id());
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
