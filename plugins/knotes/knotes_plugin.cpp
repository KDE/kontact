/*
  This file is part of Kontact
  Copyright (c) 2002 Daniel Molkentin <molkentin@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#include "knotes_plugin.h"
#include "knotes_part.h"
#include "knotes/apps/knotes_options.h"
#include "knotes/utils/knoteutils.h"
#include "summarywidget.h"
#include "knotes/knotesglobalconfig.h"
#include <KCalUtils/ICalDrag>
#include <KCalUtils/VCalDrag>
#include <KCalCore/FileStorage>


#include "kdepim-version.h"

#include <libkdepim/misc/maillistdrag.h>
using namespace KPIM;
using namespace KCalUtils;
using namespace KCalCore;

#include <KABC/VCardDrag>

#include <KontactInterface/Core>

#include <KAboutData>
#include <KCmdLineArgs>
#include <KAction>
#include <KActionCollection>
#include <KDebug>
#include <KIcon>
#include <KLocalizedString>
#include <KMessageBox>
#include <KSystemTimeZones>
#include <KTemporaryFile>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDropEvent>

EXPORT_KONTACT_PLUGIN( KNotesPlugin, knotes )

KNotesPlugin::KNotesPlugin( KontactInterface::Core *core, const QVariantList & )
    : KontactInterface::Plugin( core, core, "knotes" ),
      mAboutData( 0 )
{
    KNoteUtils::migrateToAkonadi();
    setComponentData( KontactPluginFactory::componentData() );

    KAction *action =
            new KAction( KIcon( QLatin1String("knotes") ),
                         i18nc( "@action:inmenu", "New Popup Note..." ), this );
    actionCollection()->addAction( QLatin1String("new_note"), action );
    connect( action, SIGNAL(triggered(bool)), SLOT(slotNewNote()) );
    action->setShortcut( QKeySequence( Qt::CTRL + Qt::SHIFT + Qt::Key_N ) );
    action->setHelpText(
                i18nc( "@info:status", "Create new popup note" ) );
    action->setWhatsThis(
                i18nc( "@info:whatsthis",
                       "You will be presented with a dialog where you can create a new popup note." ) );
    insertNewAction( action );

    mUniqueAppWatcher = new KontactInterface::UniqueAppWatcher(
      new KontactInterface::UniqueAppHandlerFactory<KNotesUniqueAppHandler>(), this );

}

KNotesPlugin::~KNotesPlugin()
{
}

bool KNotesPlugin::isRunningStandalone() const
{
    return mUniqueAppWatcher->isRunningStandalone();
}

QString KNotesPlugin::tipFile() const
{
    // TODO: tips file
    //QString file = KStandardDirs::locate("data", "knotes/tips");
    QString file;
    return file;
}

KParts::ReadOnlyPart *KNotesPlugin::createPart()
{
    return new KNotesPart( this );
}

KontactInterface::Summary *KNotesPlugin::createSummaryWidget( QWidget *parentWidget )
{
    return new KNotesSummaryWidget( this, parentWidget );
}

const KAboutData *KNotesPlugin::aboutData() const
{
    if ( !mAboutData ) {
        mAboutData =
                new KAboutData( "knotes", 0,
                                ki18nc( "@title", "KNotes" ),
                                KDEPIM_VERSION,
                                ki18nc( "@title", "Popup Notes" ),
                                KAboutData::License_GPL_V2,
                                ki18nc( "@info:credit", "Copyright © 2003–2013 Kontact authors" ) );

        mAboutData->addAuthor( ki18nc( "@info:credit", "Laurent Montel" ),
                               ki18nc( "@info:credit", "Current Maintainer" ),
                               "montel@kde.org" );
 
        mAboutData->addAuthor( ki18nc( "@info:credit", "Michael Brade" ),
                               ki18nc( "@info:credit", "Previous Maintainer" ),
                               "brade@kde.org" );
        mAboutData->addAuthor( ki18nc( "@info:credit", "Tobias Koenig" ),
                               ki18nc( "@info:credit", "Developer" ),
                               "tokoe@kde.org" );
    }

    return mAboutData;
}

bool KNotesPlugin::canDecodeMimeData( const QMimeData *mimeData ) const
{
    return
            mimeData->hasText() ||
            MailList::canDecode( mimeData ) ||
            KABC::VCardDrag::canDecode( mimeData ) ||
            ICalDrag::canDecode( mimeData );
}

void KNotesPlugin::processDropEvent( QDropEvent *event )
{
    const QMimeData *md = event->mimeData();

    if ( KABC::VCardDrag::canDecode( md ) ) {
        KABC::Addressee::List contacts;

        KABC::VCardDrag::fromMimeData( md, contacts );

        KABC::Addressee::List::ConstIterator it;

        QStringList attendees;
        KABC::Addressee::List::ConstIterator end(contacts.constEnd());
        for ( it = contacts.constBegin(); it != end; ++it ) {
            const QString email = (*it).fullEmail();
            if ( email.isEmpty() ) {
                attendees.append( (*it).realName() + QLatin1String("<>") );
            } else {
                attendees.append( email );
            }
        }
        event->accept();
        static_cast<KNotesPart *>( part() )->newNote(
                    i18nc( "@item", "Meeting" ), attendees.join( QLatin1String(", ") ) );
        return;
    }

    MemoryCalendar::Ptr cal( new MemoryCalendar( QString::fromLatin1( "UTC" ) ) );
    if ( ICalDrag::canDecode( md ) ) {
        KTemporaryFile tmp;
        tmp.setPrefix( QLatin1String("incidences-") );
        tmp.setSuffix( QLatin1String(".ics") );
        tmp.setAutoRemove( false );
        tmp.open();
        FileStorage storage( cal, tmp.fileName() );
        storage.save();
#if 0
        static_cast<KNotesPart *>( part() )->
                newNote( i18nc( "@item", "Note: %1", j->summary() ), j->description() );
#endif
    }
    if ( md->hasText() ) {
        static_cast<KNotesPart *>( part() )->newNote(
                    i18nc( "@item", "New Note" ), md->text() );
        return;
    }

    if ( MailList::canDecode( md ) ) {
        MailList mails = MailList::fromMimeData( md );
        event->accept();
        if ( mails.count() != 1 ) {
            KMessageBox::sorry(
                        core(),
                        i18nc( "@info", "Dropping multiple mails is not supported." ) );
        } else {
            MailSummary mail = mails.first();
            const QString txt = i18nc( "@item", "From: %1\nTo: %2\nSubject: %3",
                                 mail.from(), mail.to(), mail.subject() );
            static_cast<KNotesPart *>( part() )->newNote(
                        i18nc( "@item", "Mail: %1", mail.subject() ), txt );
        }
        return;
    }

    kWarning() << QString::fromLatin1( "Cannot handle drop events of type '%1'." ).arg( QLatin1String(event->format()) );
}

// private slots

void KNotesPlugin::slotNewNote()
{
    if ( part() ) {
        static_cast<KNotesPart *>( part() )->newNote();
    }
}

void KNotesUniqueAppHandler::loadCommandLineOptions()
{
    KCmdLineArgs::addCmdLineOptions( knotesOptions() );
}

int KNotesUniqueAppHandler::newInstance()
{
    kDebug() ;
    // Ensure part is loaded
    (void)plugin()->part();
    return KontactInterface::UniqueAppHandler::newInstance();
}
