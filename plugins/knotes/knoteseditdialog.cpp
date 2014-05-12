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

#include "knoteseditdialog.h"
#include "knotes/knoteedit.h"

#include <KActionCollection>
#include <KComponentData>
#include <KDialog>
#include <KLocalizedString>
#include <KToolBar>
#include <QLineEdit>
#include <KGlobal>

#include <KXMLGUIBuilder>
#include <KXMLGUIFactory>

#include <QAction>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>
#include <QDebug>
#include <KSharedConfig>


KNoteEditDialog::KNoteEditDialog(bool readOnly, QWidget *parent)
    : KDialog(parent)
{
    init(readOnly);
}

void KNoteEditDialog::init(bool readOnly)
{
    setCaption( readOnly ? i18nc( "@title:window", "Show Popup Note" ) : i18nc( "@title:window", "Edit Popup Note" ) );
    setButtons( readOnly ? Close : Ok | Cancel );
    setDefaultButton( readOnly ? Close : Ok );
    setModal( true );
    showButtonSeparator( true );
    // this dialog is modal to prevent one from editing the same note twice
    // in two different windows

    //QT5 setComponentData( KComponentData( "knotes" ) ); // TODO: memleak
    setXMLFile( QLatin1String("knotesui.rc") );

    QWidget *page = new QWidget( this );
    setMainWidget( page );
    QVBoxLayout *layout = new QVBoxLayout( page );

    QHBoxLayout *hbl = new QHBoxLayout();
    layout->addItem( hbl );
    hbl->setSpacing( marginHint() );
    QLabel *label = new QLabel( page );
    label->setText( i18nc( "@label popup note name", "Name:" ) );
    hbl->addWidget( label, 0 );
    mTitleEdit= new QLineEdit( page );
    mTitleEdit->setClearButtonEnabled(true);
    mTitleEdit->setObjectName( QLatin1String("name") );
    if (!readOnly)
        connect(mTitleEdit, SIGNAL(textChanged(QString)), this, SLOT(slotTextChanged(QString)));
    hbl->addWidget( mTitleEdit, 1, Qt::AlignVCenter );

    //TODO customize it
    mNoteEdit = new KNoteEdit( QLatin1String("knotesrc"), actionCollection(), page );
    mNoteEdit->setFocus();

    KXMLGUIBuilder builder( page );
    KXMLGUIFactory factory( &builder, this );
    factory.addClient( this );

    mTool = static_cast<KToolBar *>( factory.container( QLatin1String("note_tool"), this ) );
    layout->addWidget( mTool );
    layout->addWidget( mNoteEdit );

    actionCollection()->addAssociatedWidget( this );
    foreach ( QAction *action, actionCollection()->actions() ) {
        action->setShortcutContext( Qt::WidgetWithChildrenShortcut );
    }
    readConfig();
    setReadOnly(readOnly);
}

KNoteEditDialog::~KNoteEditDialog()
{
    writeConfig();
}

void KNoteEditDialog::setColor(const QColor &fg, const QColor &bg)
{
    mNoteEdit->setColor( fg, bg );
}

void KNoteEditDialog::setAcceptRichText(bool b)
{
    mNoteEdit->setAcceptRichText( b );
    mTool->setVisible(b);
}

void KNoteEditDialog::setReadOnly(bool b)
{
    mNoteEdit->setEnabled( !b );
    mTool->setEnabled(!b);
    mTitleEdit->setEnabled(!b);
}

void KNoteEditDialog::readConfig()
{
    KConfigGroup grp( KSharedConfig::openConfig(), "KNoteEditDialog" );
    const QSize size = grp.readEntry( "Size", QSize(300, 200) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void KNoteEditDialog::writeConfig()
{
    KConfigGroup grp( KSharedConfig::openConfig(), "KNoteEditDialog" );
    grp.writeEntry( "Size", size() );
    grp.sync();
}

QString KNoteEditDialog::text() const
{
    return mNoteEdit->text();
}

void KNoteEditDialog::setText( const QString &text )
{
    mNoteEdit->setText( text );
}

QString KNoteEditDialog::title() const
{
    return mTitleEdit->text();
}

void KNoteEditDialog::setTitle( const QString &text )
{
    mTitleEdit->setText( text );
    if (mTitleEdit->isEnabled())
        enableButtonOk(!text.isEmpty());
}

KNoteEdit *KNoteEditDialog::noteEdit() const
{
    return mNoteEdit;
}

void KNoteEditDialog::slotTextChanged(const QString &text)
{
     enableButtonOk(!text.isEmpty());
}

void KNoteEditDialog::setTabSize(int size)
{
    mNoteEdit->setTabStop(size);
}

void KNoteEditDialog::setAutoIndentMode( bool newmode )
{
    mNoteEdit->setAutoIndentMode(newmode);
}

void KNoteEditDialog::setTextFont( const QFont &font )
{
    mNoteEdit->setTextFont(font);
}


