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

#include "profiledialog.h"
#include "profilemanager.h"

#include <kfiledialog.h>
#include <klistview.h>
#include <klocale.h>
#include <kmessagebox.h>

#include <qlayout.h>
#include <qpushbutton.h>
#include <qstring.h>

Kontact::ProfileDialog::ProfileDialog( QWidget* parent, WFlags flags ) : KDialogBase( parent, /*name=*/0, /*modal=*/true, /*caption=*/QString(), /*buttonMask=*/KDialogBase::Ok|KDialogBase::Close )
{
    setWFlags( flags );
    setCaption( i18n("Configure Profiles") );
    setButtonOK( i18n("Load Profile") );

    QWidget* mainWidget = new QWidget( this );

    QHBoxLayout* horizontalLayout = new QHBoxLayout( mainWidget );
    horizontalLayout->setSpacing( 5 );

    m_list = new KListView( mainWidget );
    m_list->addColumn( i18n("Name") );
    m_list->addColumn( i18n("Description") );
    m_list->setSelectionMode( QListView::Single );
    m_list->setItemsRenameable( true );
    m_list->setRenameable( NameColumn, true );
    m_list->setRenameable( DescriptionColumn, true );

    connect( m_list, SIGNAL( selectionChanged() ), 
             this, SLOT( listSelectionChanged() ) );
    connect( m_list, SIGNAL( itemRenamed( QListViewItem*, const QString&, int ) ),
             this, SLOT( listItemRenamed( QListViewItem*, const QString&, int ) ) );
    horizontalLayout->addWidget( m_list );

    QVBoxLayout* buttonLayout = new QVBoxLayout( horizontalLayout );
    buttonLayout->setSpacing( 5 );

    m_newProfileButton = new QPushButton( mainWidget );
    m_newProfileButton->setText( i18n("New Profile") );
    connect( m_newProfileButton, SIGNAL( clicked() ),
             this, SLOT( addNewProfile() ) );
    buttonLayout->addWidget( m_newProfileButton );

    m_deleteProfileButton = new QPushButton( mainWidget );
    m_deleteProfileButton->setText( i18n("Delete Profile") );
    m_deleteProfileButton->setEnabled( false );
    connect( m_deleteProfileButton, SIGNAL( clicked() ),
             this, SLOT( deleteSelectedProfile() ) );
    buttonLayout->addWidget( m_deleteProfileButton );

    m_saveProfileButton = new QPushButton( mainWidget );
    m_saveProfileButton->setText( i18n("Save Profile") );
    m_saveProfileButton->setEnabled( false );
    connect( m_saveProfileButton, SIGNAL( clicked() ),
             this, SLOT( saveToSelectedProfile() ) );
    buttonLayout->addWidget( m_saveProfileButton );

    buttonLayout->addStretch();

    m_importProfileButton = new QPushButton( mainWidget );
    m_importProfileButton->setText( i18n("Import Profile") );
    connect( m_importProfileButton, SIGNAL( clicked() ),
             this, SLOT( importProfile() ) );
    buttonLayout->addWidget( m_importProfileButton );

    m_exportProfileButton = new QPushButton( mainWidget );
    m_exportProfileButton->setText( i18n("Export Profile") );
    m_exportProfileButton->setEnabled( false );
    connect( m_exportProfileButton, SIGNAL( clicked() ),
             this, SLOT( exportSelectedProfile() ) );
    buttonLayout->addWidget( m_exportProfileButton );

    setMainWidget( mainWidget );

    connect( Kontact::ProfileManager::self(), SIGNAL( profileAdded( const QString& ) ), 
             this, SLOT( profileAdded( const QString& ) ) );
    connect( Kontact::ProfileManager::self(), SIGNAL( profileRemoved( const QString& ) ), 
             this, SLOT( profileRemoved( const QString& ) ) );
    connect( Kontact::ProfileManager::self(), SIGNAL( profileLoaded( const QString& ) ), 
             this, SLOT( profileLoaded( const QString& ) ) );
    connect( Kontact::ProfileManager::self(), SIGNAL( profileUpdated( const QString& ) ), 
             this, SLOT( profileUpdated( const QString& ) ) );

    const QValueList<Kontact::Profile> profiles = Kontact::ProfileManager::self()->profiles();
    for ( QValueList<Kontact::Profile>::ConstIterator it = profiles.begin(), end = profiles.end(); it != end; ++it )
    {
        profileAdded( (*it).id() );
    }
    updateButtonState();
}

void Kontact::ProfileDialog::slotOk()
{
    loadSelectedProfile();
    KDialogBase::slotOk();
}

QString Kontact::ProfileDialog::selectedProfile() const
{
    return m_itemToProfile[m_list->selectedItem()];
}

void Kontact::ProfileDialog::loadSelectedProfile()
{
    const Kontact::Profile profile = Kontact::ProfileManager::self()->profileById( selectedProfile() );
    if ( profile.isNull() )
        return;
    Kontact::ProfileManager::self()->loadProfile( profile.id() );
}

void Kontact::ProfileDialog::profileLoaded( const QString& id )
{
    const Kontact::Profile profile = Kontact::ProfileManager::self()->profileById( id );
    if ( profile.isNull() )
        return;
    KMessageBox::information( this, i18n("The profile \"%1\" was successfully loaded. Some profile settings require a restart to get activated.").arg( profile.name() ), i18n("Profile Loaded") );
}

void Kontact::ProfileDialog::saveToSelectedProfile()
{
    const Kontact::Profile profile = Kontact::ProfileManager::self()->profileById( selectedProfile() );
    if ( profile.isNull() )
        return;
    if ( KMessageBox::Yes != KMessageBox::warningYesNo( this, i18n("The profile \"%1\" will be overwritten with the current settings. Are you sure?").arg( profile.name() ), i18n("Save to Profile"), KStdGuiItem::overwrite(), KStdGuiItem::cancel() ) )
        return;
    Kontact::ProfileManager::self()->saveToProfile( profile.id() );
}

void Kontact::ProfileDialog::deleteSelectedProfile()
{
    const Kontact::Profile profile = Kontact::ProfileManager::self()->profileById( selectedProfile() );
    if ( profile.isNull() )
        return;
    if ( KMessageBox::Yes != KMessageBox::warningYesNo( this, i18n("Do you really want to delete the profile \"%1\"? All profile settings will be lost!").arg( profile.name() ), i18n("Delete Profile"), KStdGuiItem::del(), KStdGuiItem::cancel() ) )
        return;
    Kontact::ProfileManager::self()->removeProfile( profile );
}

void Kontact::ProfileDialog::exportSelectedProfile()
{
    const QString id = selectedProfile();
    const Kontact::Profile profile = Kontact::ProfileManager::self()->profileById( id );
    if ( profile.isNull() )
        return;
    const QString path = KFileDialog::getExistingDirectory( QString(), this, i18n("Select Profile Folder") );
    if ( path.isNull() )
        return;
    const Kontact::ProfileManager::ExportError error = Kontact::ProfileManager::self()->exportProfileToDirectory( id, path );
    if ( error == Kontact::ProfileManager::SuccessfulExport )
    {
        KMessageBox::information( this, i18n("The profile \"%1\" was successfully exported.").arg( profile.name() ), i18n("Profile Exported") );
    }
    else
    {
        // TODO print error
    }
}

void Kontact::ProfileDialog::importProfile()
{
    const QString path = KFileDialog::getExistingDirectory( QString(), this, i18n("Select Profile Folder") );
    if ( path.isNull() )
        return;
    const Kontact::ProfileManager::ImportError error = Kontact::ProfileManager::self()->importProfileFromDirectory( path );
    if ( error != Kontact::ProfileManager::SuccessfulImport )
    {
        // TODO print error
    }
}

void Kontact::ProfileDialog::profileAdded( const QString& id )
{
    Q_ASSERT( !m_profileToItem[id] );
    const Kontact::Profile profile = Kontact::ProfileManager::self()->profileById( id );
    Q_ASSERT( !profile.isNull() );
    QListViewItem* const item = new QListViewItem( m_list );
    m_profileToItem[id] = item;
    m_itemToProfile[item] = id;
    profileUpdated( id );
}

void Kontact::ProfileDialog::profileRemoved( const QString& id )
{
    QListViewItem* item = m_profileToItem[id];
    Q_ASSERT( item );
    m_profileToItem.remove( id );
    m_itemToProfile.remove( item );
    delete item;
}

void Kontact::ProfileDialog::profileUpdated( const QString& id )
{
    QListViewItem* item = m_profileToItem[id];
    Q_ASSERT( item );
    const Kontact::Profile profile = Kontact::ProfileManager::self()->profileById( id );
    Q_ASSERT( !profile.isNull() );
    item->setText( NameColumn, profile.name() );
    item->setText( DescriptionColumn, profile.description() );
}

void Kontact::ProfileDialog::addNewProfile()
{
    Kontact::Profile profile( Kontact::ProfileManager::self()->generateNewId(), true );
    profile.setName( i18n("New profile") );
    profile.setDescription( i18n("Enter description") );
    Kontact::ProfileManager::self()->addProfile( profile );
}

void Kontact::ProfileDialog::listItemRenamed( QListViewItem* item, const QString& text, int col )
{
    Kontact::Profile profile = Kontact::ProfileManager::self()->profileById( m_itemToProfile[item] );
    Q_ASSERT( !profile.isNull() );
    switch ( col )
    {
        case NameColumn:
            profile.setName( text );
            Kontact::ProfileManager::self()->updateProfile( profile );
            break;
        case DescriptionColumn:
            profile.setDescription( text );
            Kontact::ProfileManager::self()->updateProfile( profile );
            break;
    }
}

void Kontact::ProfileDialog::updateButtonState()
{
    const bool hasSelection = m_list->selectedItem() != 0;
    m_deleteProfileButton->setEnabled( hasSelection );
    m_saveProfileButton->setEnabled( hasSelection);
    actionButton( KDialogBase::Ok )->setEnabled( hasSelection );
    m_exportProfileButton->setEnabled( hasSelection );
}

void Kontact::ProfileDialog::listSelectionChanged()
{
    updateButtonState();
}

#include "profiledialog.moc"
