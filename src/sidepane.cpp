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
  
  mActive = false;
  mId = id;
  mPlugin = plugin;

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
  emit showPart( mPlugin );

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

  mActive = true;

  kdDebug(5600) << "PanelButton::setActive()" << endl;
}

void PanelButton::setInactive()
{
  // reset using parents palette 
  setPalette(parentWidget()->palette());

  mActive = false;
}

void PanelButton::setPixmap(const QPixmap& pix)
{
  mPix = pix;
  QPushButton::setPixmap(pix);
}

void PanelButton::setText(const QString& text)
{
  mText = text;
  QPushButton::setText(text);
}

void PanelButton::composeLabel(QPainter *p)
{
  QRect rect = style().subRect(QStyle::SR_PushButtonContents, this);
  QRect pixRect = mPix.rect();
  pixRect.moveCenter(rect.center());
  
  if (kapp->reverseLayout())
    pixRect.setLeft(rect.right()-pixRect.width());  
  else
    pixRect.setLeft(rect.left());

  pixRect.setWidth(mPix.width());
  
  p->drawPixmap(pixRect, mPix);
  QPen tmp = p->pen();
  p->setPen(colorGroup().buttonText());
  if (kapp->reverseLayout())
  {
    rect.setRight(rect.right()-(mPix.width()+2));
    p->drawText(rect, AlignVCenter|AlignRight, mText);
  }
  else
  {
    rect.setLeft(mPix.width()+2);
    p->drawText(rect, AlignVCenter, mText);
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
    mContentStack( 0 ),
    mHeaderWidget( 0 )
{

  setSpacing(0);

  mHeaderWidget = new QLabel(this, "header");
  mHeaderWidget->setAlignment( AlignVCenter );
  mHeaderWidget->setPaletteBackgroundColor( colorGroup().dark() );
  mHeaderWidget->setPaletteForegroundColor( colorGroup().light() );
  mHeaderWidget->setFixedHeight(22);
  
  QFont fnt(font());
  fnt.setBold(true);
  fnt.setPointSize(font().pointSize()+3);
  mHeaderWidget->setFont(fnt);

  mContentStack = new QWidgetStack(this);
  mContentStack->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
  mContentStack->addWidget(new QWidget(mContentStack));
}

SidePane::~SidePane()
{
  QValueList<QGuardedPtr<QWidget> >::Iterator it;
  for ( it = mContentList.begin(); it != mContentList.end(); ++it ) {
    if ( (*it) )
      (*it)->reparent( 0, 0, QPoint( 0, 0 ) );
  }
}

void SidePane::switchSidePaneWidget( Kontact::Plugin *plugin )
{
  KParts::Part *part = plugin->part();

  Q_ASSERT(part);

  QObjectList *l = part->queryList( "KParts::SideBarExtension" );
  KParts::SideBarExtension *sbe = 0;
  if ( l )
    sbe = static_cast<KParts::SideBarExtension*>(l->first());
  delete l;

  if (!sbe) {
    mContentStack->raiseWidget(0);
    return;
  }

  if (mContentStack->id(sbe->widget()) == -1) {
    mContentStack->addWidget(sbe->widget());
    QGuardedPtr<QWidget> ptr = sbe->widget();
    mContentList.append( ptr );
  }

  mContentStack->raiseWidget(sbe->widget());
}

void SidePane::switchItems(PanelButton* pb)
{
  QPtrListIterator<PanelButton> it( mButtonList );
  for (; it.current(); ++it)
  {
    if (it.current()->isActive())
      it.current()->setInactive();
  }

  mContentStack->raiseWidget( pb->id() );
  mHeaderWidget->setText( pb->text() );
}

void SidePane::updatePlugins()
{
  // delete all existing buttons
  mButtonList.setAutoDelete( true );
  mButtonList.clear();
  mButtonList.setAutoDelete( false );

  QValueList<Plugin*> plugins = core()->pluginList();
  QValueList<Plugin*>::ConstIterator end = plugins.end();
  QValueList<Plugin*>::ConstIterator it = plugins.begin();
  for ( ; it != end; ++it ) {
    Plugin *plugin = *it;
    if ( !plugin->showInSideBar() )
      continue;

    PanelButton* pb = new PanelButton( plugin, 0, this, "PanelButton" );
    mButtonList.append( pb );
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

  QPtrListIterator<PanelButton> it( mButtonList );

  PanelButton *btn;
  while ( ( btn = it.current() ) != 0 ) {
    ++it;
    if ( btn->plugin() == plugin ) {
      btn->slotClicked();
      blockSignals( blocked );
      return;
    }
  }

  btn = mButtonList.first();

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

  QPtrListIterator<PanelButton> it( mButtonList );

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

  btn = mButtonList.first();

  // no plugins loaded. Something is really broken..
  Q_ASSERT( btn );
  if ( btn )
    btn->slotClicked();

  blockSignals( blocked );
}

#include "sidepane.moc"

// vim: sw=2 sts=2 et tw=80
