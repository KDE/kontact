/*
   This file is part of KDE Kontact.

   Copyright (C) 2003 Sven Lüppken <sven@kde.org>

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

#ifndef SUMMARYVIEW_PART_H
#define SUMMARYVIEW_PART_H

#include <kparts/part.h>

#include "core.h"

namespace Kontact
{
  class Plugin;
}

class QFrame;
class QGridLayout;

class SummaryViewPart : public KParts::ReadOnlyPart
{
  Q_OBJECT

  public:
    SummaryViewPart( Kontact::Core *core, const char *widgetName,
                     QObject *parent = 0, const char *name = 0 );
    ~SummaryViewPart();

  public slots:
    void slotTextChanged();

  signals:
    void textChanged( const QString& );

  protected:
    virtual bool openFile();
    void getWidgets();

  private:
    QGridLayout *mLayout;
    QFrame *mFrame;
    Kontact::Core *mCore;
};

#endif
