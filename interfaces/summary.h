/*
   This file is part of KDE Kontact.

   Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

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
#ifndef KONTACT_SUMMARY_H
#define KONTACT_SUMMARY_H

#include <qwidget.h>

namespace Kontact
{

/**
  Summary widget for display in the Summary View plugin.
 */
class Summary : public QWidget
{
  public:
    Summary( QWidget *parent, const char *name = 0 )
      : QWidget( parent, name )
    {
    }

    /**
      Return logical height of summary widget. This is used to calculate how
      much vertical space relative to other summary widgets this widget will use
      in the summary view.
    */
    virtual int summaryHeight() { return 1; }

  private:
    class Private;
    Private *d;
};

}

#endif
