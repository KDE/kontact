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

#ifndef KNOTESSELECTDELETENOTESDIALOG_H
#define KNOTESSELECTDELETENOTESDIALOG_H

#include <KDialog>
#include <QListWidget>
class KNotesIconViewItem;

class KNotesSelectDeleteNotesListWidget : public QListWidget
{
    Q_OBJECT
public:
    explicit KNotesSelectDeleteNotesListWidget(QWidget *parent=0);
    ~KNotesSelectDeleteNotesListWidget();

    void setItems(const QList<KNotesIconViewItem*> &items);
};

class KNotesSelectDeleteNotesDialog : public KDialog
{
    Q_OBJECT
public:
    explicit KNotesSelectDeleteNotesDialog(const QList<KNotesIconViewItem *> &items, QWidget *parent=0);
    ~KNotesSelectDeleteNotesDialog();

private:
    void writeConfig();
    void readConfig();
    KNotesSelectDeleteNotesListWidget *mSelectedListWidget;
};

#endif // KNOTESSELECTDELETENOTESDIALOG_H
