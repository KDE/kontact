/*
  This file is part of the KDE project

  Copyright (C) 2002-2003 Daniel Molkentin <molkentin@kde.org>
  Copyright (C) 2004-2006 Michael Brade <brade@kde.org>
  Copyright (C) 2013-2014 Laurent Montel <montel@kde.org>

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
#include "notesharedglobalconfig.h"
#include "noteshared/noteutils.h"
#include "knoteseditdialog.h"
#include "knotesadaptor.h"
#include "noteshared/job/createnewnotejob.h"
#include "knotesiconview.h"
#include "knoteswidget.h"
#include "knotesselectdeletenotesdialog.h"
#include "knotetip.h"
#include "knotes/configdialog/knoteconfigdialog.h"
#include "knotes/print/knoteprinter.h"
#include "knotes/print/knoteprintobject.h"
#include "knotes/print/knoteprintselectthemedialog.h"
#include "knotes/knoteedit.h"
#include "knotes/knotesglobalconfig.h"
#include "knotes/configdialog/knotesimpleconfigdialog.h"
#include "knotes/finddialog/knotefinddialog.h"
#include "utils/knoteutils.h"
#include "alarms/notealarmdialog.h"
#include "noteshared/resources/localresourcecreator.h"
#include "noteshared/job/createnewnotejob.h"

#include "noteshared/akonadi/notesakonaditreemodel.h"
#include "noteshared/akonadi/noteschangerecorder.h"
#include "noteshared/attributes/notealarmattribute.h"
#include "noteshared/attributes/showfoldernotesattribute.h"

#include "akonadi_next/note.h"

#include <Akonadi/Session>
#include <Akonadi/ChangeRecorder>
#include <Akonadi/ETMViewStateSaver>
#include <Akonadi/EntityDisplayAttribute>
#include <Akonadi/ItemCreateJob>
#include <KCheckableProxyModel>
#include <akonadi/itemdeletejob.h>


#include <KMime/KMimeMessage>

#include <Akonadi/ItemModifyJob>
#include <Akonadi/Control>

#include <KActionCollection>
#include <KAction>
#include <KInputDialog>
#include <KMessageBox>
#include <KXMLGUIFactory>
#include <KPrintPreview>
#include <ksocketfactory.h>
#include <KApplication>
#include <KFileDialog>
#include <KToggleAction>

#include <QApplication>
#include <QClipboard>
#include <QMenu>
#include <QPointer>
#include <QCheckBox>

#include <dnssd/publicservice.h>

KNotesPart::KNotesPart( QObject *parent )
    : KParts::ReadOnlyPart( parent ),
      mNotesWidget( 0 ) ,
      mPublisher(0),
      mNotePrintPreview(0),
      mNoteTreeModel(0)
{
    Akonadi::Control::widgetNeedsAkonadi(widget());

    KNoteUtils::migrateToAkonadi();

    if (KNotesGlobalConfig::self()->autoCreateResourceOnStart()) {
        NoteShared::LocalResourceCreator *creator = new NoteShared::LocalResourceCreator( this );
        creator->createIfMissing();
    }

    (void) new KNotesAdaptor( this );
    QDBusConnection::sessionBus().registerObject( QLatin1String("/KNotes"), this );

    setComponentData( KComponentData( "knotes" ) );

    // create the actions
    mNewNote = new KAction( KIcon( QLatin1String("knotes") ),
                            i18nc( "@action:inmenu create new popup note", "&New" ), this );
    actionCollection()->addAction( QLatin1String("file_new"), mNewNote );
    connect( mNewNote, SIGNAL(triggered(bool)), SLOT(newNote()) );
    mNewNote->setShortcut( QKeySequence( Qt::CTRL + Qt::Key_N ) );
    mNewNote->setHelpText(
                i18nc( "@info:status", "Create a new popup note" ) );
    mNewNote->setWhatsThis(
                i18nc( "@info:whatsthis",
                       "You will be presented with a dialog where you can add a new popup note." ) );

    mNoteEdit = new KAction( KIcon( QLatin1String("document-edit") ),
                          i18nc( "@action:inmenu", "Edit..." ), this );
    actionCollection()->addAction( QLatin1String("edit_note"), mNoteEdit );
    connect( mNoteEdit, SIGNAL(triggered(bool)), SLOT(editNote()) );
    mNoteEdit->setHelpText(
                i18nc( "@info:status", "Edit popup note" ) );
    mNoteEdit->setWhatsThis(
                i18nc( "@info:whatsthis",
                       "You will be presented with a dialog where you can modify an existing popup note." ) );

    mNoteRename = new KAction( KIcon( QLatin1String("edit-rename") ),
                          i18nc( "@action:inmenu", "Rename..." ), this );
    mNoteRename->setShortcut( QKeySequence( Qt::Key_F2 ) );
    actionCollection()->addAction( QLatin1String("edit_rename"), mNoteRename );
    connect( mNoteRename, SIGNAL(triggered(bool)), SLOT(renameNote()) );
    mNoteRename->setHelpText(
                i18nc( "@info:status", "Rename popup note" ) );
    mNoteRename->setWhatsThis(
                i18nc( "@info:whatsthis",
                       "You will be presented with a dialog where you can rename an existing popup note." ) );

    mNoteDelete = new KAction( KIcon( QLatin1String("edit-delete") ),
                          i18nc( "@action:inmenu", "Delete" ), this );
    actionCollection()->addAction( QLatin1String("edit_delete"), mNoteDelete );
    connect( mNoteDelete, SIGNAL(triggered(bool)), SLOT(killSelectedNotes()) );
    mNoteDelete->setShortcut( QKeySequence( Qt::Key_Delete ) );
    mNoteDelete->setHelpText(
                i18nc( "@info:status", "Delete popup note" ) );
    mNoteDelete->setWhatsThis(
                i18nc( "@info:whatsthis",
                       "You will be prompted if you really want to permanently remove "
                       "the selected popup note." ) );

    mNotePrint = new KAction( KIcon( QLatin1String("document-print") ),
                          i18nc( "@action:inmenu", "Print Selected Notes..." ), this );
    actionCollection()->addAction( QLatin1String("print_note"), mNotePrint );
    connect( mNotePrint, SIGNAL(triggered(bool)), SLOT(slotPrintSelectedNotes()) );
    mNotePrint->setHelpText(
                i18nc( "@info:status", "Print popup note" ) );
    mNotePrint->setWhatsThis(
                i18nc( "@info:whatsthis",
                       "You will be prompted to print the selected popup note." ) );

    if(KPrintPreview::isAvailable()) {
        mNotePrintPreview = new KAction( KIcon( QLatin1String("document-print-preview") ),i18nc( "@action:inmenu", "Print Preview Selected Notes..." ), this );
        actionCollection()->addAction( QLatin1String("print_preview_note"), mNotePrintPreview );

        connect( mNotePrintPreview, SIGNAL(triggered(bool)), SLOT(slotPrintPreviewSelectedNotes()) );
    }

    mNoteConfigure  = new KAction( KIcon( QLatin1String("configure") ), i18n( "Note settings..." ), this );
    actionCollection()->addAction( QLatin1String("configure_note"), mNoteConfigure );
    connect( mNoteConfigure, SIGNAL(triggered(bool)), SLOT(slotNotePreferences()) );

    KAction *act  = new KAction( KIcon( QLatin1String("configure") ), i18n( "Preferences KNotes..." ), this );
    actionCollection()->addAction( QLatin1String("knotes_configure"), act );
    connect( act, SIGNAL(triggered(bool)), SLOT(slotPreferences()) );

    mNoteSendMail = new KAction( KIcon( QLatin1String("mail-send") ), i18n( "Mail..." ), this );
    actionCollection()->addAction( QLatin1String("mail_note"), mNoteSendMail );
    connect( mNoteSendMail, SIGNAL(triggered(bool)), SLOT(slotMail()) );

    mNoteSendNetwork  = new KAction( KIcon( QLatin1String("network-wired") ), i18n( "Send..." ), this );
    actionCollection()->addAction( QLatin1String("send_note"), mNoteSendNetwork );
    connect( mNoteSendNetwork, SIGNAL(triggered(bool)), SLOT(slotSendToNetwork()) );

    mNoteSetAlarm  = new KAction( KIcon( QLatin1String("knotes_alarm") ), i18n( "Set Alarm..." ), this );
    actionCollection()->addAction( QLatin1String("set_alarm"), mNoteSetAlarm );
    connect( mNoteSetAlarm, SIGNAL(triggered(bool)), SLOT(slotSetAlarm()) );

    act  = new KAction( KIcon( QLatin1String("edit-paste") ),
                           i18n( "New Note From Clipboard" ), this );
    actionCollection()->addAction( QLatin1String("new_note_clipboard"), act );
    connect( act, SIGNAL(triggered()), SLOT(slotNewNoteFromClipboard()) );


    mSaveAs  = new KAction( KIcon( QLatin1String("document-save-as") ), i18n( "Save As..." ), this );
    actionCollection()->addAction( QLatin1String("save_note"), mSaveAs );
    connect( mSaveAs, SIGNAL(triggered(bool)), SLOT(slotSaveAs()) );

    mReadOnly  = new KToggleAction( KIcon( QLatin1String("object-locked") ), i18n( "Lock" ), this );
    actionCollection()->addAction( QLatin1String("lock_note"), mReadOnly );
    connect( mReadOnly, SIGNAL(triggered(bool)), SLOT(slotUpdateReadOnly()) );
    mReadOnly->setCheckedState( KGuiItem( i18n( "Unlock" ), QLatin1String("object-unlocked") ) );

    KStandardAction::find( this, SLOT(slotOpenFindDialog()), actionCollection());

    Akonadi::Session *session = new Akonadi::Session( "KNotes Session", this );
    mNoteRecorder = new NoteShared::NotesChangeRecorder(this);
    mNoteRecorder->changeRecorder()->setSession(session);
    mNoteTreeModel = new NoteShared::NotesAkonadiTreeModel(mNoteRecorder->changeRecorder(), this);

    connect( mNoteTreeModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
             SLOT(slotRowInserted(QModelIndex,int,int)));

    connect( mNoteRecorder->changeRecorder(), SIGNAL(itemChanged(Akonadi::Item,QSet<QByteArray>)), SLOT(slotItemChanged(Akonadi::Item,QSet<QByteArray>)));
    connect( mNoteRecorder->changeRecorder(), SIGNAL(itemRemoved(Akonadi::Item)), SLOT(slotItemRemoved(Akonadi::Item)) );

    mSelectionModel = new QItemSelectionModel( mNoteTreeModel );
    mModelProxy = new KCheckableProxyModel( this );
    mModelProxy->setSelectionModel( mSelectionModel );
    mModelProxy->setSourceModel( mNoteTreeModel );

    KSharedConfigPtr _config = KSharedConfig::openConfig( QLatin1String("kcmknotessummaryrc") );

    mModelState =
            new KViewStateMaintainer<Akonadi::ETMViewStateSaver>( _config->group( "CheckState" ), this );
    mModelState->setSelectionModel( mSelectionModel );

    mNotesWidget = new KNotesWidget(this,widget());
    mNoteTip = new KNoteTip( mNotesWidget->notesView() );

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
    updateNetworkListener();
}

KNotesPart::~KNotesPart()
{
    delete mPublisher;
    mPublisher=0;
    delete mNoteTip;
    mNoteTip = 0;
}

void KNotesPart::slotItemRemoved(const Akonadi::Item &item)
{
    KNotesIconViewItem *iconView = mNotesWidget->notesView()->iconView(item.id());
    delete iconView;
}

void KNotesPart::slotRowInserted(const QModelIndex &parent, int start, int end)
{
    for ( int i = start; i <= end; ++i) {
        if ( mNoteTreeModel->hasIndex( i, 0, parent ) ) {
            const QModelIndex child = mNoteTreeModel->index( i, 0, parent );
            Akonadi::Collection parentCollection = mNoteTreeModel->data( child, Akonadi::EntityTreeModel::ParentCollectionRole).value<Akonadi::Collection>();
            if (parentCollection.hasAttribute<NoteShared::ShowFolderNotesAttribute>()) {
                Akonadi::Item item =
                        mNoteTreeModel->data( child, Akonadi::EntityTreeModel::ItemRole ).value<Akonadi::Item>();
                if ( !item.hasPayload<KMime::Message::Ptr>() )
                    continue;
                mNotesWidget->notesView()->addNote(item);
            }
        }
    }
}

QStringList KNotesPart::notesList() const
{
    QStringList notes;
    QHashIterator<Akonadi::Item::Id, KNotesIconViewItem*> i(mNotesWidget->notesView()->noteList());
    while ( i.hasNext() ) {
        i.next();
        notes.append(QString::number(i.key()));
    }
    return notes;
}

void KNotesPart::requestToolTip( const QModelIndex &index )
{
    const QRect m_itemRect = mNotesWidget->notesView()->visualRect( index );
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
            listPrintObj.append(new KNotePrintObject(static_cast<KNotesIconViewItem *>( item )->item()));
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

void KNotesPart::newNote( const QString &name, const QString &text )
{
    NoteShared::CreateNewNoteJob *job = new NoteShared::CreateNewNoteJob(this, widget());
    job->setRichText(KNotesGlobalConfig::self()->richText());
    job->setNote(name, text);
    job->start();
}

void KNotesPart::slotNoteCreationFinished(KJob* job)
{
    if (job->error()) {
        kWarning() << job->errorString();
        NoteShared::NoteSharedGlobalConfig::self()->setDefaultFolder(-1);
        NoteShared::NoteSharedGlobalConfig::self()->writeConfig();
        KMessageBox::error(widget(), i18n("Note was not created."), i18n("Create new note"));
        return;
    }
}

void KNotesPart::newNoteFromClipboard( const QString &name )
{
    const QString &text = QApplication::clipboard()->text();
    newNote( name, text );
}

void KNotesPart::killNote( Akonadi::Item::Id id )
{
    killNote( id, false );
}

void KNotesPart::killNote( Akonadi::Item::Id id, bool force )
{
    KNotesIconViewItem *note = mNotesWidget->notesView()->iconView(id);
    if ( note &&
         ( (!force && KMessageBox::warningContinueCancelList(
                mNotesWidget,
                i18nc( "@info", "Do you really want to delete this note?" ),
                QStringList( note->realName() ),
                i18nc( "@title:window", "Confirm Delete" ),
                KStandardGuiItem::del() ) == KMessageBox::Continue )
           || force ) ) {

            Akonadi::ItemDeleteJob *job = new Akonadi::ItemDeleteJob(note->item());
            connect(job, SIGNAL(result(KJob*)), SLOT(slotDeleteNotesFinished(KJob*)) );
    }
}

QString KNotesPart::name( Akonadi::Item::Id id ) const
{
    KNotesIconViewItem *note = mNotesWidget->notesView()->iconView(id);
    if ( note ) {
        return note->text();
    } else {
        return QString();
    }
}

QString KNotesPart::text( Akonadi::Item::Id id ) const
{
    //TODO return plaintext ?
    KNotesIconViewItem *note = mNotesWidget->notesView()->iconView( id );
    if ( note ) {
        return note->description();
    } else {
        return QString();
    }
}

void KNotesPart::setName(Akonadi::Entity::Id id, const QString &newName )
{
    KNotesIconViewItem *note = mNotesWidget->notesView()->iconView(id);
    if ( note ) {
        note->setIconText( newName );
    }
}

void KNotesPart::setText( Akonadi::Item::Id id, const QString &newText )
{
    KNotesIconViewItem *note = mNotesWidget->notesView()->iconView( id );
    if ( note ) {
        note->setDescription( newText );
    }
}

QMap<QString, QString> KNotesPart::notes() const
{
    QMap<QString, QString> notes;
    QHashIterator<Akonadi::Item::Id, KNotesIconViewItem*> i(mNotesWidget->notesView()->noteList());
    while ( i.hasNext() ) {
        i.next();
        notes.insert(QString::number(i.key()), i.value()->realName());
    }
    return notes;
}

// private stuff

void KNotesPart::killSelectedNotes()
{
    QList<QListWidgetItem *> lst = mNotesWidget->notesView()->selectedItems();
    if ( lst.isEmpty() ) {
        return;
    }
    QList<KNotesIconViewItem*> items;

    foreach ( QListWidgetItem *item, lst ) {
        KNotesIconViewItem *knivi = static_cast<KNotesIconViewItem *>( item );
        items.append( knivi );
    }

    if (items.isEmpty())
        return;
    QPointer<KNotesSelectDeleteNotesDialog> dlg = new KNotesSelectDeleteNotesDialog(items, widget());
    if (dlg->exec()) {
        Akonadi::Item::List lst;
        QListIterator<KNotesIconViewItem*> kniviIt( items );
        while ( kniviIt.hasNext() ) {
            KNotesIconViewItem *iconViewIcon = kniviIt.next();
            if (!iconViewIcon->readOnly()) {
                lst.append(iconViewIcon->item());
            }
        }
        if (!lst.isEmpty()) {
            Akonadi::ItemDeleteJob *job = new Akonadi::ItemDeleteJob(lst);
            connect(job, SIGNAL(result(KJob*)), SLOT(slotDeleteNotesFinished(KJob*)) );
        }
    }
    delete dlg;
}

void KNotesPart::slotDeleteNotesFinished(KJob* job)
{
    if ( job->error() ) {
        qDebug()<<" problem during delete job note:"<<job->errorString();
    }
}

void KNotesPart::popupRMB( QListWidgetItem *item, const QPoint &pos, const QPoint &globalPos )
{
    Q_UNUSED( item );

    QMenu *contextMenu = new QMenu(widget());
    if ( mNotesWidget->notesView()->itemAt ( pos ) ) {
        contextMenu->addAction(mNewNote);
        const bool uniqueNoteSelected = (mNotesWidget->notesView()->selectedItems().count() == 1);
        const bool readOnly = uniqueNoteSelected ? static_cast<KNotesIconViewItem *>(mNotesWidget->notesView()->selectedItems().at(0))->readOnly() : false;
        if (uniqueNoteSelected) {
            if (!readOnly) {
                contextMenu->addSeparator();
                contextMenu->addAction(mNoteSetAlarm);
            }
            contextMenu->addSeparator();
            contextMenu->addAction(mSaveAs);
            contextMenu->addSeparator();
            contextMenu->addAction(mNoteEdit);
            contextMenu->addAction(mReadOnly);
            if (!readOnly)
                contextMenu->addAction(mNoteRename);
            contextMenu->addSeparator();
            contextMenu->addAction(mNoteSendMail);
            contextMenu->addSeparator();
            contextMenu->addAction(mNoteSendNetwork);
        }
        contextMenu->addSeparator();
        contextMenu->addAction(mNotePrint);
        contextMenu->addAction(mNotePrintPreview);

        if (!readOnly) {
            contextMenu->addSeparator();
            contextMenu->addAction(mNoteConfigure);
            contextMenu->addSeparator();
            contextMenu->addAction(mNoteDelete);
        }
    } else {
        contextMenu->addAction(mNewNote);
    }

    contextMenu->exec( mNotesWidget->notesView()->mapFromParent( globalPos ) );
    delete contextMenu;
}

void KNotesPart::editNote(Akonadi::Entity::Id id)
{
    KNotesIconViewItem *knoteItem = mNotesWidget->notesView()->iconView(id);
    if (knoteItem) {
        mNotesWidget->notesView()->setCurrentItem(knoteItem);
        editNote(knoteItem);
    }
}

void KNotesPart::editNote( QListWidgetItem *item )
{
    KNotesIconViewItem * knotesItem = static_cast<KNotesIconViewItem *>( item );
    QPointer<KNoteEditDialog> dlg = new KNoteEditDialog( knotesItem->readOnly(), widget() );
    dlg->setTitle( knotesItem->realName() );
    dlg->setText( knotesItem->description() );

    dlg->setAcceptRichText(knotesItem->isRichText());
    dlg->setTabSize(knotesItem->tabSize());
    dlg->setAutoIndentMode(knotesItem->autoIndent());
    dlg->setTextFont(knotesItem->textFont());

    dlg->noteEdit()->setFocus();
    if ( dlg->exec() == QDialog::Accepted ) {
        //Don't save we save in description
        knotesItem->setIconText( dlg->title(), false );
        knotesItem->setDescription( dlg->text() );
    }
    delete dlg;
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

    const QString oldName = knoteItem->realName();
    bool ok = false;
    const QString newName =
            KInputDialog::getText( i18nc( "@title:window", "Rename Popup Note" ),
                                   i18nc( "@label:textbox", "New Name:" ),
                                   oldName, &ok, mNotesWidget );
    if ( ok && ( newName != oldName ) ) {
        knoteItem->setIconText( newName );
    }
}

void KNotesPart::slotOnCurrentChanged( )
{
    const bool uniqueNoteSelected = (mNotesWidget->notesView()->selectedItems().count() == 1);
    const bool enabled(mNotesWidget->notesView()->currentItem());
    mNoteRename->setEnabled( enabled && uniqueNoteSelected);
    mNoteEdit->setEnabled( enabled && uniqueNoteSelected);
    mNoteConfigure->setEnabled( uniqueNoteSelected );
    mNoteSendMail->setEnabled(uniqueNoteSelected);
    mNoteSendNetwork->setEnabled(uniqueNoteSelected);
    mNoteSetAlarm->setEnabled(uniqueNoteSelected);
    mSaveAs->setEnabled(uniqueNoteSelected);
    mReadOnly->setEnabled(uniqueNoteSelected);
    if (uniqueNoteSelected) {
        const bool readOnly = static_cast<KNotesIconViewItem *>(mNotesWidget->notesView()->selectedItems().at(0))->readOnly();
        mReadOnly->setChecked(readOnly);
        mNoteEdit->setText(readOnly ? i18n("Show Note...") : i18nc( "@action:inmenu", "Edit..." ));
    } else {
        mNoteEdit->setText(i18nc( "@action:inmenu", "Edit..." ));
    }
}

void KNotesPart::slotNotePreferences()
{
    if (!mNotesWidget->notesView()->currentItem())
        return;
    KNotesIconViewItem *knoteItem = static_cast<KNotesIconViewItem *>(mNotesWidget->notesView()->currentItem());
    QPointer<KNoteSimpleConfigDialog> dialog = new KNoteSimpleConfigDialog( knoteItem->realName(), widget() );
    Akonadi::Item item = knoteItem->item();
    dialog->load(item, knoteItem->isRichText());
    if (dialog->exec() ) {
        KNoteUtils::updateConfiguration();
        bool isRichText;
        dialog->save(item, isRichText);
        KMime::Message::Ptr message = item.payload<KMime::Message::Ptr>();
        message->contentType( true )->setMimeType( isRichText ? "text/html" : "text/plain" );
        message->assemble();
        Akonadi::ItemModifyJob *job = new Akonadi::ItemModifyJob(item);
        connect( job, SIGNAL(result(KJob*)), SLOT(slotNoteSaved(KJob*)) );
    }
    delete dialog;
}

void KNotesPart::slotPreferences()
{
    // create a new preferences dialog...
    KNoteConfigDialog *dialog = new KNoteConfigDialog( i18n( "Settings" ), widget());
    connect( dialog, SIGNAL(configWrote()), this, SLOT(slotConfigUpdated()));
    dialog->show();
}

void KNotesPart::updateConfig()
{
    updateNetworkListener();
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
    NoteShared::NoteUtils::sendToMail(widget(),knoteItem->realName(), knoteItem->description());
}

void KNotesPart::slotSendToNetwork()
{
    if (!mNotesWidget->notesView()->currentItem())
        return;
    KNotesIconViewItem *knoteItem = static_cast<KNotesIconViewItem *>(mNotesWidget->notesView()->currentItem());
    NoteShared::NoteUtils::sendToNetwork(widget(), knoteItem->realName(), knoteItem->description());
}

void KNotesPart::updateNetworkListener()
{
    delete mPublisher;
    mPublisher=0;

    if ( NoteShared::NoteSharedGlobalConfig::receiveNotes() ) {
        // create the socket and start listening for connections
        mPublisher=new DNSSD::PublicService(NoteShared::NoteSharedGlobalConfig::senderID(), QLatin1String("_knotes._tcp"), NoteShared::NoteSharedGlobalConfig::port());
        mPublisher->publishAsync();
    }
}

void KNotesPart::slotSetAlarm()
{
    if (!mNotesWidget->notesView()->currentItem())
        return;
    KNotesIconViewItem *knoteItem = static_cast<KNotesIconViewItem *>(mNotesWidget->notesView()->currentItem());
    QPointer<NoteShared::NoteAlarmDialog> dlg = new NoteShared::NoteAlarmDialog( knoteItem->realName(), widget() );
    Akonadi::Item item = knoteItem->item();
    if (item.hasAttribute<NoteShared::NoteAlarmAttribute>()) {
        dlg->setAlarm(item.attribute<NoteShared::NoteAlarmAttribute>()->dateTime());
    }
    if ( dlg->exec() ) {
        bool needToModify = true;
        KDateTime dateTime = dlg->alarm();
        if (dateTime.isValid()) {
            NoteShared::NoteAlarmAttribute *attribute =  item.attribute<NoteShared::NoteAlarmAttribute>( Akonadi::Entity::AddIfMissing );
            attribute->setDateTime(dateTime);
        } else {
            if (item.hasAttribute<NoteShared::NoteAlarmAttribute>()) {
                item.removeAttribute<NoteShared::NoteAlarmAttribute>();
            } else {
                needToModify = false;
            }
        }
        if (needToModify) {
            Akonadi::ItemModifyJob *job = new Akonadi::ItemModifyJob(item);
            connect( job, SIGNAL(result(KJob*)), SLOT(slotNoteSaved(KJob*)) );
        }
    }
    delete dlg;
}

void KNotesPart::slotNoteSaved(KJob *job)
{
    qDebug()<<" void KNote::slotNoteSaved(KJob *job)";
    if ( job->error() ) {
        qDebug()<<" problem during save note:"<<job->errorString();
    }
}

void KNotesPart::slotNewNoteFromClipboard()
{
    const QString &text = KApplication::clipboard()->text();
    newNote( QString(), text );
}

void KNotesPart::slotSaveAs()
{
    if (!mNotesWidget->notesView()->currentItem())
        return;
    KNotesIconViewItem *knoteItem = static_cast<KNotesIconViewItem *>(mNotesWidget->notesView()->currentItem());

    KUrl url;
    QCheckBox *convert = 0;

    if ( knoteItem->isRichText() ) {
        convert = new QCheckBox( 0 );
        convert->setText( i18n( "Save note as plain text" ) );
    }
    QPointer<KFileDialog> dlg = new KFileDialog( url, QString(), widget(), convert );
    dlg->setOperationMode( KFileDialog::Saving );
    dlg->setCaption( i18n( "Save As" ) );
    if( !dlg->exec() ) {
        delete dlg;
        return;
    }

    const QString fileName = dlg->selectedFile();
    const bool htmlFormatAndSaveAsHtml = (convert && !convert->isChecked());
    delete dlg;
    if ( fileName.isEmpty() ) {
        return;
    }

    QFile file( fileName );
    if ( file.exists() &&
         KMessageBox::warningContinueCancel( widget(),
                                             i18n( "<qt>A file named <b>%1</b> already exists.<br />"
                                                   "Are you sure you want to overwrite it?</qt>",
                                                   QFileInfo( file ).fileName() ) ) != KMessageBox::Continue ) {
        return;
    }

    if ( file.open( QIODevice::WriteOnly ) ) {
        QTextStream stream( &file );
        QTextDocument doc;
        doc.setHtml(knoteItem->description());
        if ( htmlFormatAndSaveAsHtml ) {
            QString htmlStr = doc.toHtml();
            htmlStr.replace(QLatin1String("meta name=\"qrichtext\" content=\"1\""), QLatin1String("meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\""));
            stream <<  htmlStr;
        } else {
            stream << knoteItem->realName() + QLatin1Char('\n');
            stream << doc.toPlainText();
        }
    }
}

void KNotesPart::slotUpdateReadOnly()
{
    QListWidgetItem *item = mNotesWidget->notesView()->currentItem();
    if (!item)
        return;
    KNotesIconViewItem *knoteItem = static_cast<KNotesIconViewItem *>(item);

    const bool readOnly = mReadOnly->isChecked();

    mNoteEdit->setText(readOnly ? i18n("Show Note...") : i18nc( "@action:inmenu", "Edit..." ));
    knoteItem->setReadOnly( readOnly );
}

void KNotesPart::slotItemChanged(const Akonadi::Item &item, const QSet<QByteArray> & set)
{
    KNotesIconViewItem *knoteItem = mNotesWidget->notesView()->iconView(item.id());
    if (knoteItem) {
        knoteItem->setChangeItem(item, set);
    }
}

void KNotesPart::slotOpenFindDialog()
{
    if (!mNoteFindDialog) {
        mNoteFindDialog = new KNoteFindDialog(widget());
        connect(mNoteFindDialog, SIGNAL(noteSelected(Akonadi::Item::Id)), this, SLOT(slotSelectNote(Akonadi::Item::Id)));
    }
    QHash<Akonadi::Item::Id , Akonadi::Item> lst;
    QHashIterator<Akonadi::Item::Id, KNotesIconViewItem*> i(mNotesWidget->notesView()->noteList());
    while (i.hasNext()) {
        i.next();
        lst.insert(i.key(), i.value()->item());
    }
    mNoteFindDialog->setExistingNotes(lst);
    mNoteFindDialog->show();
}

void KNotesPart::slotSelectNote(Akonadi::Item::Id id)
{
    editNote(id);
}
