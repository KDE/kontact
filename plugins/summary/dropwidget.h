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
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef DROP_WIDGET_H
#define DROP_WIDGET_H

#include <qwidget.h>

class DropWidget : public QWidget
{
  Q_OBJECT

  public:
    DropWidget( QWidget *parent, const char *name = 0 );

  signals:
    void summaryWidgetDropped( QWidget *target, QWidget *widget, int alignment );

  protected:
    virtual void dragEnterEvent( QDragEnterEvent* );
    virtual void dropEvent( QDropEvent* );
};

#endif
