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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include "summary.h"

#include <qhbox.h>
#include <qlabel.h>
#include <qfont.h>

#include <kiconloader.h>
#include <kdialog.h>

using namespace Kontact;

Summary::Summary( QWidget *parent, const char *name )
  : QWidget( parent, name )
{
}

Summary::~Summary()
{
}

QWidget* Summary::createHeader(QWidget *parent, const QPixmap& icon, const QString& heading)
{
  QHBox* hbox = new QHBox( parent );

  QFont boldFont;
  boldFont.setBold( true );
  boldFont.setPointSize( boldFont.pointSize() + 2 );

  QLabel *label = new QLabel( hbox );
  label->setPixmap( icon );
  label->setFixedSize( label->sizeHint() );
  label->setPaletteBackgroundColor( colorGroup().mid() );

  label = new QLabel( heading, hbox );
  label->setAlignment( AlignLeft|AlignVCenter );
  label->setIndent( KDialog::spacingHint() );
  label->setFont( boldFont );
  label->setPaletteForegroundColor( colorGroup().light() );
  label->setPaletteBackgroundColor( colorGroup().mid() );

  hbox->setPaletteBackgroundColor( colorGroup().mid() );

  return hbox;
}


#include "summary.moc"
