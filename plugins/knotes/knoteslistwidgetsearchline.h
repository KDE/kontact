/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef KNOTESLISTWIDGETSEARCHLINE_H
#define KNOTESLISTWIDGETSEARCHLINE_H

#include <KListWidgetSearchLine>

class KNotesListWidgetSearchLine : public KListWidgetSearchLine
{
public:
    explicit KNotesListWidgetSearchLine(QWidget *parent=0);
    ~KNotesListWidgetSearchLine();

    void updateClickMessage(const QString &shortcutStr);

protected:
    bool itemMatches( const QListWidgetItem *item, const QString &s ) const;

};

#endif // KNOTESLISTWIDGETSEARCHLINE_H
