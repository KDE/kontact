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

#include <qsignal.h>
#include <qpainter.h>
#include <qdrawutil.h>

#include <kglobal.h>
#include <kiconloader.h>

#include "navigator.h"


EntryItem::EntryItem( QListBox *parent, const QPixmap& icon, const QString& name,
                      QObject* receiver, const QCString& slot )
  : QListBoxPixmap(KIconLoader::unknown() ) {
  m_slot = slot;
  m_receiver = receiver;
  m_parent = parent;
  m_Pixmap = new QPixmap(icon);
  setCustomHighlighting( true );
  setText( name );
}

EntryItem::~EntryItem() {
}

void EntryItem::connectToReceiver(QObject* sender) {
  QSignal sig(sender);
  sig.connect(m_receiver, m_slot);
  sig.activate();
}

int EntryItem::width( const QListBox *listbox) const {
  return listbox->viewport()->width();
}

int EntryItem::height( const QListBox *listbox) const {
  int min = 0;
  min = listbox->fontMetrics().lineSpacing() + pixmap()->height() + 6;
  return min;
}

void EntryItem::paint(QPainter *p)
{
  QListBox *box = listBox();
  int w = width( box );
  static const int margin = 3;
  int y = margin;
  const QPixmap *pm = pixmap();

  if ( !pm->isNull() ) {
    int x = (w - pm->width()) / 2;
    x = QMAX( x, margin );
    p->drawPixmap( x, y, *pm );
  }

  if ( !text().isEmpty() ) {
    QFontMetrics fm = p->fontMetrics();
    y += pm->height() + fm.height() - fm.descent();
    int x = (w - fm.width( text() )) / 2;
    x = QMAX( x, margin );
    p->drawText( x, y, text() );
  }
  // draw sunken
  if ( isCurrent() || isSelected() ) {
    qDrawShadePanel( p, 1, 0, w -2, height(box),
                     box->colorGroup(), true, 1, 0L );
  }
}


Navigator::Navigator(QWidget *parent, const char *name)
  : KListBox(parent, name)
{

  setSelectionMode( KListBox::Single );
  QPalette pal = palette();
  QColor gray = pal.color(QPalette::Normal, QColorGroup::Mid );
  pal.setColor( QPalette::Normal, QColorGroup::Base, gray );
  pal.setColor( QPalette::Inactive, QColorGroup::Base, gray );

  setPalette( pal );
  viewport()->setBackgroundMode( PaletteMid);


  connect(this, SIGNAL(currentChanged(QListBoxItem *)), this, SLOT(slotExecuted(QListBoxItem *)));
}

QSize Navigator::sizeHint() const
{
	return QSize(100, 100);
}

void Navigator::addEntry(QString text, QString icon, QObject *receiver, const char *slot)
{
  QPixmap pixmap = KGlobal::iconLoader()->loadIcon(icon, KIcon::Desktop, 48);
  insertItem(new EntryItem(this, pixmap, text, receiver, slot));
}

void Navigator::slotExecuted(QListBoxItem *item)
{
  static_cast<EntryItem*>(item)->connectToReceiver(this);
}


#include "navigator.moc"

// vim: ts=4 sw=4 et
