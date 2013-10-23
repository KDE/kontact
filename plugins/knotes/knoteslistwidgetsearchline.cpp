/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include "knoteslistwidgetsearchline.h"

#include "knotesiconview.h"

#include <KLocale>

#include <KCal/Journal>
using namespace KCal;


KNotesListWidgetSearchLine::KNotesListWidgetSearchLine(QWidget *parent)
    : KListWidgetSearchLine(parent)
{
    setClickMessage(i18n("Search notes..."));
}

KNotesListWidgetSearchLine::~KNotesListWidgetSearchLine()
{

}

bool KNotesListWidgetSearchLine::itemMatches( const QListWidgetItem *item, const QString &s ) const
{
    if (!item) {
        return false;
    }
    const KNotesIconViewItem *iconView = dynamic_cast<const KNotesIconViewItem *>( item );
    if (!iconView) {
        return false;
    }
    Journal *journal = iconView->journal();
    if (!journal) {
        return false;
    }
    if (journal->summary().contains(s)) {
        return true;
    }
    if (journal->description().contains(s)) {
        return true;
    }
    return KListWidgetSearchLine::itemMatches(item, s);
}
