/*
   This file is part of KDE Kontact.

   Copyright (C) 2004 Tobias Koenig <tokoe@kde.org>

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

#include <qdragobject.h>

#include "dropwidget.h"

DropWidget::DropWidget( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  setAcceptDrops( true );
}

void DropWidget::dragEnterEvent( QDragEnterEvent *event )
{
  event->accept( QTextDrag::canDecode( event ) );
}

void DropWidget::dropEvent( QDropEvent *event )
{
  int alignment = ( event->pos().x() < (width() / 2) ? Qt::AlignLeft : Qt::AlignRight );
  alignment |= ( event->pos().y() < (height() / 2) ? Qt::AlignTop : Qt::AlignBottom );
  emit summaryWidgetDropped( this, event->source(), alignment );
}

#include "dropwidget.moc"
