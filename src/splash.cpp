/*
   This file is part of the KDE project
   Copyright (C) 2002 Daniel Molkentin <molkentin@kde.org>

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

#include <qcursor.h>
#include <qdesktopwidget.h>
#include <qlabel.h>
#include <qprogressbar.h>

#include <kapplication.h>
#include <kglobalsettings.h>
#include <kiconloader.h>

#include "splash.h"

using namespace Kontact;

Splash::Splash( QWidget *parent, const char *name )
  : QVBox( parent, name, WStyle_Customize | WStyle_Splash )
{
  QLabel *lbl = new QLabel( this );
  QPixmap splash( UserIcon( "splash" ) );
  lbl->setBackgroundPixmap( splash );
  mProgressBar = new QProgressBar( this );
  resize( splash.size() );
  setFrameStyle( QFrame::Box | QFrame::Plain );

  QRect desk = KGlobalSettings::splashScreenDesktopGeometry();

  setGeometry( ( desk.width() / 2 ) - ( width() / 2 ) + desk.left(),
               ( desk.height() / 2 ) - ( height() / 2 ) + desk.top(),
               width(), height() );
}

#include "splash.moc"
