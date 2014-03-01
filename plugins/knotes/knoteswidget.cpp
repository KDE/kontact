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

#include "knoteswidget.h"
#include "knotesiconview.h"
#include "knoteslistwidgetsearchline.h"

#include <QVBoxLayout>

KNotesWidget::KNotesWidget(KNotesPart *part, QWidget *parent)
    : QWidget(parent)
{
    QVBoxLayout *lay = new QVBoxLayout;
    mSearchLine = new KNotesListWidgetSearchLine;
    lay->addWidget(mSearchLine);
    mIconView = new KNotesIconView(part, parent);
    mSearchLine->setListWidget(mIconView);
    lay->addWidget(mIconView);
    setLayout(lay);
}

KNotesWidget::~KNotesWidget()
{

}

void KNotesWidget::slotFocusQuickSearch()
{
    mSearchLine->setFocus();
}

void KNotesWidget::updateClickMessage(const QString &shortcutStr)
{
    mSearchLine->updateClickMessage(shortcutStr);
}

KNotesIconView *KNotesWidget::notesView() const
{
    return mIconView;
}

