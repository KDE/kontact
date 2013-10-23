/*
  This file is part of the KDE project

  Copyright (C) 2002-2003 Daniel Molkentin <molkentin@kde.org>
  Copyright (C) 2004-2006 Michael Brade <brade@kde.org>
  Copyright (C) 2013 Laurent Montel <montel@kde.org>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; see the file COPYING.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "knotes_part.h"
#include "knoteseditdialog.h"
#include "knotesadaptor.h"
#include "knotesiconview.h"
#include "knoteswidget.h"
#include "knotetip.h"
#include "knotes/knoteconfigdlg.h"
#include "knotes/network/knotesnetrecv.h"
#include "knotes/print/knoteprinter.h"
#include "knotes/print/knoteprintobject.h"
#include "knotes/print/knoteprintselectthemedialog.h"
#include "knotes/resource/resourcemanager.h"
#include "knotes/knoteedit.h"
#include "knotes/knotesglobalconfig.h"
#include "knotesimpleconfigdialog.h"
#include "knoteutils.h"
#include "knotealarmdlg.h"
#include "knotesalarm.h"
#include <KCal/Journal>
using namespace KCal;

#include <KActionCollection>
#include <KAction>
#include <KInputDialog>
#include <KMessageBox>
#include <KXMLGUIFactory>
#include <KPrintPreview>
#include <ksocketfactory.h>

#include <QApplication>
#include <QClipboard>
#include <QTcpServer>
#include <QMenu>

#include <dnssd/publicservice.h>

KNotesPart::KNotesPart( KNotesResourceManager *manager, QObject *parent )
    : KParts::ReadOnlyPart( parent ),
      mNotesWidget( new KNotesWidget(this) ),
      mNoteTip( new KNoteTip( mNotesWidget->notesView() ) ),
      mNoteEditDlg( 0 ),
      mManager( manager ),
      mListener(0),
      mPublisher(0),
      mAlarm(0)
{
    (void) new KNotesAdaptor( this );
    QDBusConnection::sessionBus().registerObject( QLatin1String("/KNotes"), this );

    setComponentData( KComponentData( "knotes" ) );

    // create the actions
    KAction *action =
            new KAction( KIcon( QLatin1String("knotes") ),
                         i18nc( "@action:inmenu create new popup note", "&New" ), this );
    actionCollection()->addAction( QLatin1String("file_new"), action );
    connect( action, SIGNAL(triggered(bool)), SLOT(newNote()) );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_N ) );
    action->setHelpText(
                i18nc( "@info:status", "Create a new popup note" ) );
    action->setWhatsThis(
                i18nc( "@info:whatsthis",
                       "You will be presented with a dialog where you can add a new popup note." ) );

    action = new KAction( KIcon( QLatin1String("document-edit") ),
                          i18nc( "@action:inmenu", "Edit..." ), this );
    actionCollection()->addAction( QLatin1String("edit_note"), action );
    connect( action, SIGNAL(triggered(bool)), SLOT(editNote()) );
    action->setHelpText(
                i18nc( "@info:status", "Edit popup note" ) );
    action->setWhatsThis(
                i18nc( "@info:whatsthis",
                       "You will be presented with a dialog where you can modify an existing popup note." ) );

    action = new KAction( KIcon( QLatin1String("edit-rename") ),
                          i18nc( "@action:inmenu", "Rename..." ), this );
    actionCollection()->addAction( QLatin1String("edit_rename"), action );
    connect( action, SIGNAL(triggered(bool)), SLOT(renameNote()) );
    action->setHelpText(
                i18nc( "@info:status", "Rename popup note" ) );
    action->setWhatsThis(
                i18nc( "@info:whatsthis",
                       "You will be presented with a dialog where you can rename an existing popup note." ) );

    action = new KAction( KIcon( QLatin1String("edit-delete") ),
                          i18nc( "@action:inmenu", "Delete" ), this );
    actionCollection()->addAction( QLatin1String("edit_delete"), action );
    connect( action, SIGNAL(triggered(bool)), SLOT(killSelectedNotes()) );
    action->setShortcut( QKeySequence( Qt::Key_Delete ) );
    action->setHelpText(
                i18nc( "@info:status", "Delete popup note" ) );
    action->setWhatsThis(
                i18nc( "@info:whatsthis",
                       "You will be prompted if you really want to permanently remove "
                       "the selected popup note." ) );

    action = new KAction( KIcon( QLatin1String("document-print") ),
                          i18nc( "@action:inmenu", "Print Selected Notes..." ), this );
    actionCollection()->addAction( QLatin1String("print_note"), action );
    connect( action, SIGNAL(triggered(bool)), SLOT(slotPrintSelectedNotes()) );
    action->setHelpText(
                i18nc( "@info:status", "Print popup note" ) );
    action->setWhatsThis(
                i18nc( "@info:whatsthis",
                       "You will be prompted to print the selected popup note." ) );

    if(KPrintPreview::isAvailable()) {

        action = new KAction( KIcon( QLatin1String("document-print-preview") ),i18nc( "@action:inmenu", "Print Preview Selected Notes..." ), this );
        actionCollection()->addAction( QLatin1String("print_preview_note"), action );

        connect( action, SIGNAL(triggered(bool)), SLOT(slotPrintPreviewSelectedNotes()) );
    }

    action  = new KAction( KIcon( QLatin1String("configure") ), i18n( "Note settings..." ), this );
    actionCollection()->addAction( QLatin1String("configure_note"), action );
    connect( action, SIGNAL(triggered(bool)), SLOT(slotNotePreferences()) );

    action  = new KAction( KIcon( QLatin1String("configure") ), i18n( "Preferences KNotes..." ), this );
    actionCollection()->addAction( QLatin1String("knotes_configure"), action );
    connect( action, SIGNAL(triggered(bool)), SLOT(slotPreferences()) );

    action  = new KAction( KIcon( QLatin1String("mail-send") ), i18n( "Mail..." ), this );
    actionCollection()->addAction( QLatin1String("mail_note"), action );
    connect( action, SIGNAL(triggered(bool)), SLOT(slotMail()) );

    action  = new KAction( KIcon( QLatin1String("network-wired") ), i18n( "Send..." ), this );
    actionCollection()->addAction( QLatin1String("send_note"), action );
    connect( action, SIGNAL(triggered(bool)), SLOT(slotSendToNetwork()) );

    action  = new KAction( KIcon( QLatin1String("knotes_alarm") ), i18n( "Set Alarm..." ), this );
    actionCollection()->addAction( QLatin1String("set_alarm"), action );
    connect( action, SIGNAL(triggered(bool)), SLOT(slotSetAlarm()) );


    // TODO icons: s/editdelete/knotes_delete/ or the other way round in knotes

    // set the view up

    connect( mNotesWidget->notesView(), SIGNAL(executed(QListWidgetItem*)),
             this, SLOT(editNote(QListWidgetItem*)) );

    connect( mNotesWidget->notesView(), SIGNAL(entered(QModelIndex)),
             this, SLOT(requestToolTip(QModelIndex)));

    connect( mNotesWidget->notesView(), SIGNAL(viewportEntered()),
             this, SLOT(hideToolTip()));

    connect( mNotesWidget->notesView(), SIGNAL(itemSelectionChanged()),
             this, SLOT(slotOnCurrentChanged()) );

    slotOnCurrentChanged();

    setWidget( mNotesWidget );
    setXMLFile( QLatin1String("knotes_part.rc") );

    // connect the resource manager
    connect( mManager, SIGNAL(sigRegisteredNote(KCal::Journal*)),
             this, SLOT(createNote(KCal::Journal*)) );
    connect( mManager, SIGNAL(sigDeregisteredNote(KCal::Journal*)),
             this, SLOT(killNote(KCal::Journal*)) );
    mManager->load();
    mAlarm = new KNotesAlarm( mManager, this );
    updateNetworkListener();
}

KNotesPart::~KNotesPart()
{
    delete mListener;
    mListener=0;
    delete mPublisher;
    mPublisher=0;
    delete mNoteTip;
    mNoteTip = 0;
}

void KNotesPart::requestToolTip( const QModelIndex &index )
{
    QRect m_itemRect = mNotesWidget->notesView()->visualRect( index );
    mNoteTip->setNote(
                static_cast<KNotesIconViewItem *>( mNotesWidget->notesView()->itemAt( m_itemRect.topLeft() ) ) );
}

void KNotesPart::hideToolTip()
{
    mNoteTip->setNote( 0 );
}

void KNotesPart::slotPrintPreviewSelectedNotes()
{
    printSelectedNotes(true);
}

void KNotesPart::slotPrintSelectedNotes()
{
    printSelectedNotes(false);
}

void KNotesPart::printSelectedNotes(bool preview)
{
    QList<QListWidgetItem *> lst = mNotesWidget->notesView()->selectedItems();
    if ( lst.isEmpty() ) {
        KMessageBox::information(
                    mNotesWidget,
                    i18nc( "@info",
                           "To print notes, first select the notes to print from the list." ),
                    i18nc( "@title:window", "Print Popup Notes" ) );
        return;
    }

    KNotesGlobalConfig *globalConfig = KNotesGlobalConfig::self();
    QString printingTheme = globalConfig->theme();
    if (printingTheme.isEmpty()) {
        QPointer<KNotePrintSelectThemeDialog> dlg = new KNotePrintSelectThemeDialog(widget());
        if (dlg->exec()) {
            printingTheme = dlg->selectedTheme();
        }
        delete dlg;
    }
    if (!printingTheme.isEmpty()) {

        QList<KNotePrintObject *> listPrintObj;
        foreach ( QListWidgetItem *item, lst ) {
            listPrintObj.append(new KNotePrintObject(static_cast<KNotesIconViewItem *>( item )->journal()));
        }
        KNotePrinter printer;
        printer.printNotes( listPrintObj, printingTheme, preview );
        qDeleteAll(listPrintObj);
    }
}

bool KNotesPart::openFile()
{
    return false;
}

// public KNotes D-Bus interface implementation

QString KNotesPart::newNote( const QString &name, const QString &text )
{
    // create the new note
    Journal *journal = new Journal();

    // new notes have the current date/time as title if none was given
    if ( !name.isEmpty() ) {
        journal->setSummary( name );
    } else {
        journal->setSummary( KGlobal::locale()->formatDateTime( QDateTime::currentDateTime() ) );
    }

    // the body of the note
    journal->setDescription( text );

    // Edit the new note if text is empty
    if ( text.isNull() ) {
        delete mNoteEditDlg;
        mNoteEditDlg = new KNoteEditDialog( widget() );

        mNoteEditDlg->setTitle( journal->summary() );
        mNoteEditDlg->setText( journal->description() );


        const QString property = journal->customProperty("KNotes", "RichText");
        if ( !property.isNull() ) {
            mNoteEditDlg->setAcceptRichText( property == QLatin1String("true") ? true : false );
        } else {
            KNotesGlobalConfig *globalConfig = KNotesGlobalConfig::self();
            mNoteEditDlg->setAcceptRichText( globalConfig->richText());
        }


        mNoteEditDlg->noteEdit()->setFocus();
        if ( mNoteEditDlg->exec() == QDialog::Accepted ) {
            journal->setSummary( mNoteEditDlg->title() );
            journal->setDescription( mNoteEditDlg->text() );
        } else {
            delete journal;
            return QString();
        }
    }

    mManager->addNewNote( journal );

    KNotesIconViewItem *note = mNoteList.value( journal->uid() );
    mNotesWidget->notesView()->scrollToItem( note );
    mNotesWidget->notesView()->setCurrentItem( note );
    mManager->save();
    return journal->uid();
}

QString KNotesPart::newNoteFromClipboard( const QString &name )
{
    const QString &text = QApplication::clipboard()->text();
    return newNote( name, text );
}

void KNotesPart::killNote( const QString &id )
{
    killNote( id, false );
}

void KNotesPart::killNote( const QString &id, bool force )
{
    KNotesIconViewItem *note = mNoteList.value( id );

    if ( note &&
         ( (!force && KMessageBox::warningContinueCancelList(
                mNotesWidget,
                i18nc( "@info", "Do you really want to delete this note?" ),
                QStringList( mNoteList.value( id )->text() ),
                i18nc( "@title:window", "Confirm Delete" ),
                KStandardGuiItem::del() ) == KMessageBox::Continue )
           || force ) ) {
        KNoteUtils::removeNote(mNoteList.value( id )->journal(), 0);
        mManager->deleteNote( mNoteList.value( id )->journal() );
        mManager->save();
    }
}

QString KNotesPart::name( const QString &id ) const
{
    KNotesIconViewItem *note = mNoteList.value( id );
    if ( note ) {
        return note->text();
    } else {
        return QString();
    }
}

QString KNotesPart::text( const QString &id ) const
{
    KNotesIconViewItem *note = mNoteList.value( id );
    if ( note ) {
        return note->journal()->description();
    } else {
        return QString();
    }
}

void KNotesPart::setName( const QString &id, const QString &newName )
{
    KNotesIconViewItem *note = mNoteList.value( id );
    if ( note ) {
        note->setIconText( newName );
        mManager->save();
    }
}

void KNotesPart::setText( const QString &id, const QString &newText )
{
    KNotesIconViewItem *note = mNoteList.value( id );
    if ( note ) {
        note->journal()->setDescription( newText );
        mManager->save();
    }
}

QMap<QString, QString> KNotesPart::notes() const
{
    QMap<QString, QString> notes;

    QHashIterator<QString, KNotesIconViewItem*> i(mNoteList);
    while ( i.hasNext() ) {
        i.next();
        notes.insert( i.value()->journal()->uid(), i.value()->journal()->summary() );
    }
    return notes;
}

// private stuff

void KNotesPart::killSelectedNotes()
{
    QList<KNotesIconViewItem*> items;
    QStringList notes;
    QList<QListWidgetItem *> lst = mNotesWidget->notesView()->selectedItems ();
    if ( lst.isEmpty() ) {
        return;
    }

    foreach ( QListWidgetItem *item, lst ) {
        KNotesIconViewItem *knivi = static_cast<KNotesIconViewItem *>( item );
        items.append( knivi );
        notes.append( knivi->realName() );
    }

    int ret = KMessageBox::warningContinueCancelList(
                mNotesWidget,
                i18ncp( "@info",
                        "Do you really want to delete this note?",
                        "Do you really want to delete these %1 notes?", items.count() ),
                notes, i18nc( "@title:window", "Confirm Delete" ),
                KStandardGuiItem::del() );

    if ( ret == KMessageBox::Continue ) {
        QListIterator<KNotesIconViewItem*> kniviIt( items );
        while ( kniviIt.hasNext() ) {
            Journal *journal = kniviIt.next()->journal();
            KNoteUtils::removeNote(journal, 0);
            mManager->deleteNote( journal );
        }
        mManager->save();
    }
}

void KNotesPart::popupRMB( QListWidgetItem *item, const QPoint &pos, const QPoint &globalPos )
{
    Q_UNUSED( item );

    QMenu *contextMenu = 0;
    if ( mNotesWidget->notesView()->itemAt ( pos ) ) {
        contextMenu = static_cast<QMenu *>( factory()->container( QLatin1String("note_context"), this ) );
    } else {
        contextMenu = static_cast<QMenu *>( factory()->container( QLatin1String("notepart_context"), this ) );
    }

    if ( !contextMenu ) {
        return;
    }

    contextMenu->popup( mNotesWidget->notesView()->mapFromParent( globalPos ) );
}

void KNotesPart::mouseMoveOnListWidget( const QPoint & pos )
{
    QListWidgetItem *item = mNotesWidget->notesView()->itemAt( pos );
    mNoteTip->setNote( dynamic_cast<KNotesIconViewItem *>( item ) );
}

// TODO: also with takeItem, clear(),

// create and kill the icon view item corresponding to the note, edit the note

void KNotesPart::createNote( KCal::Journal *journal )
{
    mNoteList.insert( journal->uid(), new KNotesIconViewItem( mNotesWidget->notesView(), journal ) );
}

void KNotesPart::killNote( KCal::Journal *journal )
{
    KNotesIconViewItem *item = mNoteList.take( journal->uid() );
    delete item;
}

void KNotesPart::editNote( QListWidgetItem *item )
{
    if ( !mNoteEditDlg ) {
        mNoteEditDlg = new KNoteEditDialog( widget() );
    }

    Journal *journal = static_cast<KNotesIconViewItem *>( item )->journal();
    mNoteEditDlg->setTitle( journal->summary() );
    mNoteEditDlg->setText( journal->description() );

    const QString property = journal->customProperty("KNotes", "RichText");
    if ( !property.isNull() ) {
        mNoteEditDlg->setAcceptRichText( property == QLatin1String("true") ? true : false );
    } else {
        KNotesGlobalConfig *globalConfig = KNotesGlobalConfig::self();
        mNoteEditDlg->setAcceptRichText( globalConfig->richText());
    }

    mNoteEditDlg->noteEdit()->setFocus();
    if ( mNoteEditDlg->exec() == QDialog::Accepted ) {
        static_cast<KNotesIconViewItem *>( item )->setIconText( mNoteEditDlg->title() );
        journal->setDescription( mNoteEditDlg->text() );
        mManager->save();
    }
}

void KNotesPart::editNote()
{
    QListWidgetItem *item = mNotesWidget->notesView()->currentItem();
    if ( item ) {
        editNote( item );
    }
}

void KNotesPart::renameNote()
{
    KNotesIconViewItem *knoteItem = static_cast<KNotesIconViewItem *>(mNotesWidget->notesView()->currentItem());

    QString oldName = knoteItem->realName();
    bool ok = false;
    QString newName =
            KInputDialog::getText( i18nc( "@title:window", "Rename Popup Note" ),
                                   i18nc( "@label:textbox", "New Name:" ),
                                   oldName, &ok, mNotesWidget );
    if ( ok && ( newName != oldName ) ) {
        knoteItem->setIconText( newName );
        mManager->save();
    }
}

void KNotesPart::slotOnCurrentChanged( )
{
    QAction *renameAction = actionCollection()->action( QLatin1String("edit_rename") );
    QAction *deleteAction = actionCollection()->action( QLatin1String("edit_delete") );
    QAction *editAction = actionCollection()->action( QLatin1String("edit_note") );
    QAction *configureAction = actionCollection()->action( QLatin1String("configure_note") );
    QAction *sendMailAction = actionCollection()->action( QLatin1String("mail_note") );
    QAction *sendToNetworkAction = actionCollection()->action( QLatin1String("send_note") );
    QAction *setAlarmAction = actionCollection()->action( QLatin1String("set_alarm") );

    const bool uniqueNoteSelected = (mNotesWidget->notesView()->selectedItems().count() == 1);
    const bool enabled(mNotesWidget->notesView()->currentItem());
    renameAction->setEnabled( enabled );
    deleteAction->setEnabled( enabled );
    editAction->setEnabled( enabled );
    configureAction->setEnabled( uniqueNoteSelected );
    sendMailAction->setEnabled(uniqueNoteSelected);
    sendToNetworkAction->setEnabled(uniqueNoteSelected);
    setAlarmAction->setEnabled(uniqueNoteSelected);
}

void KNotesPart::slotNotePreferences()
{
    if (!mNotesWidget->notesView()->currentItem())
        return;

    KNotesIconViewItem *knoteItem = static_cast<KNotesIconViewItem *>(mNotesWidget->notesView()->currentItem());
    const QString name = knoteItem->realName();
    QPointer<KNoteSimpleConfigDialog> dialog = new KNoteSimpleConfigDialog( knoteItem->config(), name, widget(), knoteItem->journal()->uid() );
    connect( dialog, SIGNAL(settingsChanged(QString)) , this,
             SLOT(slotApplyConfig()) );
    dialog->exec();
    delete dialog;
}

void KNotesPart::slotApplyConfig()
{
    KNotesIconViewItem *knoteItem = static_cast<KNotesIconViewItem *>(mNotesWidget->notesView()->currentItem());
    knoteItem->updateColor();
    mManager->save();
}

void KNotesPart::slotPreferences()
{
    // create a new preferences dialog...
    KNoteConfigDlg *dialog = new KNoteConfigDlg( i18n( "Settings" ), widget());
    connect( dialog, SIGNAL(configWrote()), this, SLOT(slotConfigUpdated()));
    dialog->show();
}

void KNotesPart::slotConfigUpdated()
{
    updateNetworkListener();
}

void KNotesPart::slotMail()
{
    if (!mNotesWidget->notesView()->currentItem())
        return;
    KNotesIconViewItem *knoteItem = static_cast<KNotesIconViewItem *>(mNotesWidget->notesView()->currentItem());
    KNoteUtils::sendMail(widget(),knoteItem->realName(), knoteItem->journal()->description());
}

void KNotesPart::slotSendToNetwork()
{
    if (!mNotesWidget->notesView()->currentItem())
        return;
    KNotesIconViewItem *knoteItem = static_cast<KNotesIconViewItem *>(mNotesWidget->notesView()->currentItem());
    KNoteUtils::sendToNetwork(widget(),knoteItem->realName(), knoteItem->journal()->description());
}

void KNotesPart::updateNetworkListener()
{
    delete mListener;
    mListener=0;
    delete mPublisher;
    mPublisher=0;

    if ( KNotesGlobalConfig::receiveNotes() ) {
        // create the socket and start listening for connections
        mListener = KSocketFactory::listen( QLatin1String("knotes") , QHostAddress::Any,
                                           KNotesGlobalConfig::port() );
        connect( mListener, SIGNAL(newConnection()), SLOT(slotAcceptConnection()) );
        mPublisher=new DNSSD::PublicService(KNotesGlobalConfig::senderID(), QLatin1String("_knotes._tcp"), KNotesGlobalConfig::port());
        mPublisher->publishAsync();
    }
}

void KNotesPart::slotAcceptConnection()
{
    // Accept the connection and make KNotesNetworkReceiver do the job
    QTcpSocket *s = mListener->nextPendingConnection();

    if ( s ) {
        KNotesNetworkReceiver *recv = new KNotesNetworkReceiver( s );
        connect( recv,SIGNAL(sigNoteReceived(QString,QString)), SLOT(newNote(QString,QString)) );
    }
}

void KNotesPart::slotSetAlarm()
{
    if (!mNotesWidget->notesView()->currentItem())
        return;
    KNotesIconViewItem *knoteItem = static_cast<KNotesIconViewItem *>(mNotesWidget->notesView()->currentItem());

    QPointer<KNoteAlarmDlg> dlg = new KNoteAlarmDlg( knoteItem->realName(), widget() );
    dlg->setIncidence( knoteItem->journal() );
    if ( dlg->exec() ) {
        mManager->save();
    }
    delete dlg;
}


#include "knotes_part.moc"
