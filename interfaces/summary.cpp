/*
   This file is part of KDE Kontact.

   Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>
   Copyright (c) 2003 Daniel Molkentin <molkentin@kde.org>

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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "summary.h"

#include <qimage.h>
#include <q3dragobject.h>
#include <khbox.h>
#include <qfont.h>
#include <qlabel.h>
#include <qpainter.h>
//Added by qt3to4:
#include <QPixmap>
#include <QMouseEvent>
#include <QDragEnterEvent>
#include <QDropEvent>

#include <kiconloader.h>
#include <kdialog.h>

using namespace Kontact;

Summary::Summary( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
  setAcceptDrops( true );
}

Summary::~Summary()
{
}

QWidget* Summary::createHeader(QWidget *parent, const QPixmap& icon, const QString& heading)
{
  KHBox* hbox = new KHBox( parent );
  hbox->setMargin( 2 );

  QFont boldFont;
  boldFont.setBold( true );
  boldFont.setPointSize( boldFont.pointSize() + 2 );

  QLabel *label = new QLabel( hbox );
  label->setPixmap( icon );
  label->setFixedSize( label->sizeHint() );
  label->setPaletteBackgroundColor( colorGroup().mid() );
  label->setAcceptDrops( true );

  label = new QLabel( heading, hbox );
  label->setAlignment( Qt::AlignLeft|Qt::AlignVCenter );
  label->setIndent( KDialog::spacingHint() );
  label->setFont( boldFont );
  label->setPaletteForegroundColor( colorGroup().light() );
  label->setPaletteBackgroundColor( colorGroup().mid() );

  hbox->setPaletteBackgroundColor( colorGroup().mid() );

  hbox->setMaximumHeight( hbox->minimumSizeHint().height() );

  return hbox;
}

void Summary::mousePressEvent( QMouseEvent *event )
{
  mDragStartPoint = event->pos();

  QWidget::mousePressEvent( event );
}

void Summary::mouseMoveEvent( QMouseEvent *event )
{
  if ( (event->state() & Qt::LeftButton) &&
       (event->pos() - mDragStartPoint).manhattanLength() > 4 ) {

   
    QDrag *drag = new QDrag(this);
    drag->setMimeData(new QMimeData());
    drag->setObjectName("SummaryWidgetDrag");

    QPixmap pm = QPixmap::grabWidget( this );
    if ( pm.width() > 300 )
      pm = pm.convertToImage().smoothScale( 300, 300, Qt::KeepAspectRatio );

    QPainter painter;
    painter.begin( &pm );
    painter.setPen( Qt::gray );
    painter.drawRect( 0, 0, pm.width(), pm.height() );
    painter.end();
    drag->setPixmap( pm );
    drag->start(Qt::MoveAction);
  } else
    QWidget::mouseMoveEvent( event );
}

void Summary::dragEnterEvent( QDragEnterEvent *event )
{
  if (event->source()->inherits("Kontact::Summary"))
    event->acceptProposedAction();
}

void Summary::dropEvent( QDropEvent *event )
{
  int alignment = (event->pos().y() < (height() / 2) ? Qt::AlignTop : Qt::AlignBottom);
  emit summaryWidgetDropped( this, event->source(), alignment );
}

#include "summary.moc"
