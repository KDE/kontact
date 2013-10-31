/*
  This file is part of Kontact.

  Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#ifndef SUMMARYWIDGET_H
#define SUMMARYWIDGET_H

#include <KCal/Journal>
#include <KontactInterface/Summary>

namespace KCal {
class CalendarLocal;
}
using namespace KCal;

class KNotesPlugin;
class QGridLayout;
class QLabel;

class KNotesSummaryWidget : public KontactInterface::Summary
{
    Q_OBJECT
public:
    KNotesSummaryWidget(KCal::CalendarLocal *calendar, KNotesPlugin *plugin, QWidget *parent );
    ~KNotesSummaryWidget();

    void updateSummary( bool force = false );

protected:
    virtual bool eventFilter( QObject *obj, QEvent *e );

protected slots:
    void urlClicked( const QString & );
    void updateView();

private:
    QGridLayout *mLayout;

    QList<QLabel *> mLabels;
    KNotesPlugin *mPlugin;
    QPixmap mPixmap;
    KCal::CalendarLocal *mCalendar;
};

#endif
