/*
  This file is part of KAddressBook Kontact Plugin.

  Copyright (c) 2009-2015 Laurent Montel <montel@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include "kaddressbook_plugin.h"
#include "kaddressbook_options.h"
#include "kaddressbookinterface.h"

#include <KontactInterface/Core>

#include <QAction>
#include <KActionCollection>
#include <KCmdLineArgs>
#include <QDebug>
#include <KLocalizedString>

#include <QIcon>

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusReply>
#include <QStandardPaths>

EXPORT_KONTACT_PLUGIN(KAddressBookPlugin, kaddressbook)

KAddressBookPlugin::KAddressBookPlugin(KontactInterface::Core *core, const QVariantList &)
    : KontactInterface::Plugin(core, core, "kaddressbook")
{
#pragma message("port QT5")
    //QT5 setComponentData( KontactPluginFactory::componentData() );

    QAction *action =
        new QAction(QIcon::fromTheme(QStringLiteral("contact-new")),
                    i18nc("@action:inmenu", "New Contact..."), this);
    actionCollection()->addAction(QStringLiteral("new_contact"), action);
    connect(action, &QAction::triggered, this, &KAddressBookPlugin::slotNewContact);
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_C));
    //action->setHelpText(
    //  i18nc( "@info:status", "Create a new contact" ) );
    action->setWhatsThis(
        i18nc("@info:whatsthis",
              "You will be presented with a dialog where you can create a new contact."));
    insertNewAction(action);

    action =
        new QAction(QIcon::fromTheme(QStringLiteral("user-group-new")),
                    i18nc("@action:inmenu", "New Contact Group..."), this);
    actionCollection()->addAction(QStringLiteral("new_contactgroup"), action);
    connect(action, &QAction::triggered, this, &KAddressBookPlugin::slotNewContactGroup);
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::SHIFT + Qt::Key_G));
    //action->setHelpText(
    //  i18nc( "@info:status", "Create a new contact group" ) );
    action->setWhatsThis(
        i18nc("@info:whatsthis",
              "You will be presented with a dialog where you can create a new contact group."));
    insertNewAction(action);

    QAction *syncAction =
        new QAction(QIcon::fromTheme(QStringLiteral("view-refresh")),
                    i18nc("@action:inmenu", "Sync Contacts"), this);
    actionCollection()->addAction(QStringLiteral("kaddressbook_sync"), syncAction);
    connect(syncAction, &QAction::triggered, this, &KAddressBookPlugin::slotSyncContacts);
    //syncAction->setHelpText(
    //  i18nc( "@info:status", "Synchronize groupware contacts" ) );
    syncAction->setWhatsThis(
        i18nc("@info:whatsthis",
              "Choose this option to synchronize your groupware contacts."));
    insertSyncAction(syncAction);

    mUniqueAppWatcher = new KontactInterface::UniqueAppWatcher(
        new KontactInterface::UniqueAppHandlerFactory<KAddressBookUniqueAppHandler>(), this);
}

KAddressBookPlugin::~KAddressBookPlugin()
{
}

void KAddressBookPlugin::slotNewContact()
{
    KParts::ReadOnlyPart *part = createPart();
    if (!part) {
        return;
    }

    if (part->metaObject()->indexOfMethod("newContact()") == -1) {
        qWarning() << "KAddressBook part is missing slot newContact()";
        return;
    }

    QMetaObject::invokeMethod(part, "newContact");
}

void KAddressBookPlugin::slotNewContactGroup()
{
    KParts::ReadOnlyPart *part = createPart();
    if (!part) {
        return;
    }

    if (part->metaObject()->indexOfMethod("newGroup()") == -1) {
        qWarning() << "KAddressBook part is missing slot newGroup()";
        return;
    }

    QMetaObject::invokeMethod(part, "newGroup");
}

QString KAddressBookPlugin::tipFile() const
{
    // TODO: tips file
    //QString file = QStandardPaths::locate(QStandardPaths::GenericDataLocation, "kaddressbook/tips");
    QString file;
    return file;
}

KParts::ReadOnlyPart *KAddressBookPlugin::createPart()
{
    KParts::ReadOnlyPart *part = loadPart();
    if (!part) {
        return Q_NULLPTR;
    }

    // disable the Ctrl+N shortcut, as it is used by Kontact already
    if (part->action("akonadi_contact_create")) {
        QAction *newAction = qobject_cast<QAction *>(part->action("akonadi_contact_create"));
        if (newAction) {
            newAction->setShortcut(QKeySequence());
        }
    }

    return part;
}

bool KAddressBookPlugin::isRunningStandalone() const
{
    return mUniqueAppWatcher->isRunningStandalone();
}

QStringList KAddressBookPlugin::invisibleToolbarActions() const
{
    QStringList actions;
    actions << QStringLiteral("akonadi_contact_create") << QStringLiteral("akonadi_contact_group_create");
    return actions;
}

void KAddressBookPlugin::shortcutChanged()
{
    KParts::ReadOnlyPart *localPart = part();
    if (localPart) {
        if (localPart->metaObject()->indexOfMethod("updateQuickSearchText()") == -1) {
            qWarning() << "KAddressBook part is missing slot updateQuickSearchText()";
            return;
        }
        QMetaObject::invokeMethod(localPart, "updateQuickSearchText");
    }
}

void KAddressBookPlugin::slotSyncContacts()
{
#if 0
    QDBusMessage message =
        QDBusMessage::createMethodCall("org.kde.kmail", "/Groupware",
                                       "org.kde.kmail.groupware",
                                       "triggerSync");
    message << QString("Contact");
    QDBusConnection::sessionBus().send(message);
#else
    qWarning() << QStringLiteral(" Need to port to AKONADI: KAddressBookPlugin::slotSyncNotes");
#endif
}

void KAddressBookUniqueAppHandler::loadCommandLineOptions()
{
    KCmdLineArgs::addCmdLineOptions(kaddressbook_options());
}

int KAddressBookUniqueAppHandler::newInstance()
{
    // Ensure part is loaded
    (void)plugin()->part();
    org::kde::kaddressbook kaddressbook(QStringLiteral("org.kde.kaddressbook"), QStringLiteral("/KAddressBook"), QDBusConnection::sessionBus());
    QDBusReply<bool> reply = kaddressbook.handleCommandLine();
    return KontactInterface::UniqueAppHandler::newInstance();
}

#include "kaddressbook_plugin.moc"
