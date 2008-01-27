/*
    This file is part of KDE Kontact.

    Copyright (c) 2007 Frank Osterfeld <frank.osterfeld@kdemail.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef KONTACT_PROFILEDIALOG_H
#define KONTACT_PROFILEDIALOG_H

#include <kdialogbase.h>

#include <qmap.h>
#include <qstring.h>

class QListViewItem;

class KListView;
class QPushButton;

namespace Kontact {

class ProfileDialog : public KDialogBase
{
Q_OBJECT

public:
    explicit ProfileDialog( QWidget* parent = 0, WFlags f = 0 );

private:
    enum ListColumn {
        NameColumn=0,
        DescriptionColumn=1
    };

    QString selectedProfile() const;
    void updateButtonState();

protected slots:

    //override
    void slotOk();

private slots:

    void loadSelectedProfile();
    void saveToSelectedProfile();
    void deleteSelectedProfile();
    void importProfile();
    void exportSelectedProfile();
    void addNewProfile();
    void listSelectionChanged();
    void listItemRenamed( QListViewItem* item, const QString& text, int col );

    void profileAdded( const QString& id );
    void profileRemoved( const QString& id );
    void profileUpdated( const QString& id );
    void profileLoaded( const QString& id );

private:
    KListView* m_list;
    QPushButton* m_newProfileButton;
    QPushButton* m_deleteProfileButton;
    QPushButton* m_saveProfileButton;
    QPushButton* m_importProfileButton;
    QPushButton* m_exportProfileButton;
    QMap<QListViewItem*, QString> m_itemToProfile;
    QMap<QString, QListViewItem*> m_profileToItem;
};

} // Kontact

#endif // KONTACT_PROFILEDIALOG_H
