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
#include <qlabel.h>
#include <qpainter.h>
#include <qbitmap.h>
#include <qfontmetrics.h>
#include <qstyle.h>

#include <kapplication.h>
#include <kconfig.h>
#include <klocale.h>
#include <kiconloader.h>
#include <sidebarextension.h>

#include <kdebug.h>

#include "mainwindow.h"

#include "plugin.h"

#include "sidepane.h"

using namespace Kontact;

///////////////////////////////////////////////////////////////////////
// Helper classes
///////////////////////////////////////////////////////////////////////

PanelButton::PanelButton( Kontact::Plugin *plugin, int id,  QWidget *parent, const char* name)
  : QPushButton(parent, name)
{
  
  setPixmap( BarIcon( plugin->icon() ) );
  setText( plugin->title() ); 
  
  m_active = false;
  m_id = id;
  m_plugin = plugin;

  QFont fnt(font());
  fnt.setBold(true);
  setFont(fnt);

  if (style().inherits("KStyle")) 
    setFlat(true);

  connect(this, SIGNAL(clicked()), SLOT(slotClicked()));
}

void PanelButton::slotClicked()
{
  emit clicked( this );
  emit showPart( m_plugin );

  setActive();
}

void PanelButton::setActive()
{
  QColorGroup cga(palette().active());
  cga.setColor(QColorGroup::Button, cga.highlight());
  cga.setColor(QColorGroup::ButtonText, cga.highlightedText());

  QColorGroup cgi(palette().inactive());
  cgi.setColor(QColorGroup::Button, cgi.highlight());
  cgi.setColor(QColorGroup::ButtonText, cgi.highlightedText());

  QPalette pal = palette();
  pal.setActive(cga);
  pal.setInactive(cgi);
  setPalette(pal);

  m_active = true;

  kdDebug(5600) << "PanelButton::setActive()" << endl;
}

void PanelButton::setInactive()
{
  // reset using parents palette 
  setPalette(parentWidget()->palette());

  m_active = false;
}

void PanelButton::setPixmap(const QPixmap& pix)
{
  m_pix = pix;
  QPushButton::setPixmap(pix);
}

void PanelButton::setText(const QString& text)
{
  m_text = text;
  QPushButton::setText(text);
}

void PanelButton::composeLabel(QPainter *p)
{
  QRect rect = style().subRect(QStyle::SR_PushButtonContents, this);
  QRect pixRect = m_pix.rect();
  pixRect.moveCenter(rect.center());
  
  if (kapp->reverseLayout())
    pixRect.setLeft(rect.right()-pixRect.width());  
  else
    pixRect.setLeft(rect.left());

  pixRect.setWidth(m_pix.width());
  
  p->drawPixmap(pixRect, m_pix);
  QPen tmp = p->pen();
  p->setPen(colorGroup().text());
  if (kapp->reverseLayout())
  {
    rect.setRight(rect.right()-(m_pix.width()+2));
    p->drawText(rect, AlignVCenter|AlignRight, m_text);
  }
  else
  {
    rect.setLeft(m_pix.width()+2);
    p->drawText(rect, AlignVCenter, m_text);
  }
  p->setPen(tmp);
  
}

void PanelButton::drawButtonLabel(QPainter *p)
{
  composeLabel(p);
}

///////////////////////////////////////////////////////////////////////

SidePane::SidePane( Core* core, QWidget *parent, const char* name )
  : SidePaneBase( core, parent, name ),
    m_contentStack( 0 ),
    m_headerWidget( 0 )
{

  setSpacing(0);

  m_headerWidget = new QLabel(this, "header");
  m_headerWidget->setAlignment( AlignVCenter );
  m_headerWidget->setPaletteBackgroundColor( colorGroup().dark() );
  m_headerWidget->setPaletteForegroundColor( colorGroup().light() );
  m_headerWidget->setFixedHeight(22);
  
  QFont fnt(font());
  fnt.setBold(true);
  fnt.setPointSize(font().pointSize()+3);
  m_headerWidget->setFont(fnt);

  m_contentStack = new QWidgetStack(this);
  m_contentStack->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
  m_contentStack->addWidget(new QWidget(m_contentStack));
}

SidePane::~SidePane()
{
  QValueList<QGuardedPtr<QWidget> >::Iterator it;
  for ( it = m_contentList.begin(); it != m_contentList.end(); ++it ) {
    if ( (*it) )
      (*it)->reparent( 0, 0, QPoint( 0, 0 ) );
  }
}

void SidePane::switchSidePaneWidget( Kontact::Plugin *plugin )
{
  KParts::Part *part = plugin->part();

  Q_ASSERT(part);

  QObjectList *l = part->queryList( "KParts::SideBarExtension" );
  KParts::SideBarExtension *sbe = static_cast<KParts::SideBarExtension*>(l->first());

  if (!sbe) {
    m_contentStack->raiseWidget(0);
    return;
  }

  if (m_contentStack->id(sbe->widget()) == -1) {
    m_contentStack->addWidget(sbe->widget());
    QGuardedPtr<QWidget> ptr = sbe->widget();
    m_contentList.append( ptr );
  }

  m_contentStack->raiseWidget(sbe->widget());
}

void SidePane::switchItems(PanelButton* pb)
{
  QPtrListIterator<PanelButton> it( m_buttonList );
  for (; it.current(); ++it)
  {
    if (it.current()->isActive())
      it.current()->setInactive();
  }

  m_contentStack->raiseWidget( pb->id() );
  m_headerWidget->setText( pb->text() );
}

void SidePane::updatePlugins()
{
  // delete all existing buttons
  m_buttonList.setAutoDelete( true );
  m_buttonList.clear();
  m_buttonList.setAutoDelete( false );

  QPtrList<Plugin> plugins = core()->pluginList();
  for ( Plugin* plugin = plugins.first(); plugin; plugin = plugins.next() ) {
    if ( !plugin->showInSideBar() )
      continue;

    PanelButton* pb = new PanelButton( plugin, 0, this, "PanelButton" );
    m_buttonList.append( pb );
    connect( pb, SIGNAL( clicked( PanelButton* ) ),
             SLOT( switchItems( PanelButton* ) ) );
    connect( pb, SIGNAL( showPart( Kontact::Plugin* ) ),
             SIGNAL( pluginSelected( Kontact::Plugin* ) ) );
    connect( pb, SIGNAL( showPart( Kontact::Plugin* ) ), 
             SLOT( switchSidePaneWidget( Kontact::Plugin* ) ) );

    pb->show();
  }
}

void SidePane::selectPlugin( Kontact::Plugin *plugin )
{
  bool blocked = signalsBlocked();
  blockSignals( true );

  QPtrListIterator<PanelButton> it( m_buttonList );

  PanelButton *btn;
  while ( ( btn = it.current() ) != 0 ) {
    ++it;
    if ( btn->plugin() == plugin ) {
      btn->slotClicked();
      blockSignals( blocked );
      return;
    }
  }

  btn = m_buttonList.first();

  // no plugins loaded. Something is really broken..
  Q_ASSERT( btn );
  if ( btn )
    btn->slotClicked();

  blockSignals( blocked );
}

void SidePane::selectPlugin( const QString &pluginName )
{
  bool blocked = signalsBlocked();
  blockSignals( true );

  QPtrListIterator<PanelButton> it( m_buttonList );

  PanelButton *btn;
  while ( ( btn = it.current() ) != 0 ) {
    ++it;
    Kontact::Plugin *plugin = btn->plugin();
    if ( plugin->identifier() == pluginName ) {
      btn->slotClicked();
      blockSignals( blocked );
      return;
    }
  }

  btn = m_buttonList.first();

  // no plugins loaded. Something is really broken..
  Q_ASSERT( btn );
  if ( btn )
    btn->slotClicked();

  blockSignals( blocked );
}

#include "sidepane.moc"
