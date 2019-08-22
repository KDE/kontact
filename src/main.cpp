/*
  This file is part of KDE Kontact.

  Copyright (c) 2001 Matthias Hoelzer-Kluepfel <mhk@kde.org>
  Copyright (c) 2002-2003 Daniel Molkentin <molkentin@kde.org>

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

// Use the kdepim version
#include "kontact-version.h"

#include "mainwindow.h"
#include "kontact_debug.h"
#include "prefs.h"
using namespace Kontact;

#include <KdepimDBusInterfaces/ReminderClient>

#include <KontactInterface/Plugin>
#include <KontactInterface/UniqueAppHandler>
#include <KontactInterface/PimUniqueApplication>

#include <KAboutData>
#include <KLocalizedString>
#include <KService>
#include <KServiceTypeTrader>
#include <KWindowSystem>
#include <KCrash>

#include <QCommandLineParser>

#include <iostream>
#include <kdelibs4configmigrator.h>
using namespace std;

static const char version[] = KONTACT_VERSION;

class KontactApp : public KontactInterface::PimUniqueApplication
{
    Q_OBJECT
public:
    KontactApp(int &argc, char **argv[])
        : KontactInterface::PimUniqueApplication(argc, argv)
        , mMainWindow(nullptr)
        , mSessionRestored(false)
    {
        KLocalizedString::setApplicationDomain("kontact");
    }

    ~KontactApp()
    {
    }

    int activate(const QStringList &arguments, const QString &workingDir) override;

    void setMainWindow(MainWindow *window)
    {
        mMainWindow = window;
        KontactInterface::UniqueAppHandler::setMainWidget(window);
    }

    void setSessionRestored(bool restored)
    {
        mSessionRestored = restored;
    }

private:
    QPointer<MainWindow> mMainWindow;
    bool mSessionRestored;
};

static void listPlugins()
{
    const KService::List offers = KServiceTypeTrader::self()->query(
        QStringLiteral("Kontact/Plugin"),
        QStringLiteral("[X-KDE-KontactPluginVersion] == %1").arg(KONTACT_PLUGIN_VERSION));
    const KService::List::ConstIterator end(offers.end());
    for (KService::List::ConstIterator it = offers.begin(); it != end; ++it) {
        KService::Ptr service = (*it);
        // skip summary only plugins
        QVariant var = service->property(QStringLiteral("X-KDE-KontactPluginHasPart"));
        if (var.isValid() && var.toBool() == false) {
            continue;
        }
        cout << "lib name " << qPrintable(service->library().remove(QStringLiteral("kontact_"))) << endl;
    }
}

static void loadCommandLineOptions(QCommandLineParser *parser)
{
    parser->addOption(QCommandLineOption(
                          QStringLiteral("module"),
                          i18n("Start with a specific Kontact module"),
                          QStringLiteral("module")));
    parser->addOption(QCommandLineOption(
                          QStringLiteral("iconify"),
                          i18n("Start in iconified (minimized) mode")));
    parser->addOption(QCommandLineOption(
                          QStringLiteral("list"),
                          i18n("List all possible modules and exit")));
}

int KontactApp::activate(const QStringList &args, const QString &workingDir)
{
    Q_UNUSED(workingDir);

    QCommandLineParser parser;
    loadCommandLineOptions(&parser);
    parser.process(args);

    QString moduleName;
    if (Prefs::self()->forceStartupPlugin()) {
        moduleName = Prefs::self()->forcedStartupPlugin();
    }
    if (parser.isSet(QStringLiteral("module"))) {
        moduleName = parser.value(QStringLiteral("module"));
    }
    if (!mSessionRestored) {
        if (!mMainWindow) {
            mMainWindow = new MainWindow();
            if (!moduleName.isEmpty()) {
                mMainWindow->setInitialActivePluginModule(moduleName);
            }
            mMainWindow->show();
            KontactInterface::UniqueAppHandler::setMainWidget(mMainWindow);
            // --iconify is needed in kontact, although kstart can do that too,
            // because kstart returns immediately so it's too early to talk D-Bus to the app.
            if (parser.isSet(QStringLiteral("iconify"))) {
                KWindowSystem::minimizeWindow(mMainWindow->winId());
            }
        } else {
            if (!moduleName.isEmpty()) {
                mMainWindow->setInitialActivePluginModule(moduleName);
            }
        }
    } else if (!moduleName.isEmpty()) {
        mMainWindow->selectPlugin(moduleName);
    }

    KPIM::ReminderClient::startDaemon();

    // Handle startup notification and window activation
    // (The first time it will do nothing except note that it was called)
    return 0;
}

int main(int argc, char **argv)
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    KontactApp app(argc, &argv);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps, true);

    KCrash::initialize();
    Kdelibs4ConfigMigrator migrate(QStringLiteral("kontact"));
    migrate.setConfigFiles(QStringList() << QStringLiteral("kontactrc") << QStringLiteral("kontact_summaryrc"));
    migrate.setUiFiles(QStringList() << QStringLiteral("kontactui.rc"));
    migrate.migrate();

    KAboutData about(QStringLiteral("kontact"),
                     i18n("Kontact"),
                     QLatin1String(version),
                     i18n("KDE personal information manager"),
                     KAboutLicense::GPL,
                     i18n("Copyright © 2001–2019 Kontact authors"),
                     QString(),
                     QStringLiteral("https://userbase.kde.org/Kontact"));

    about.addAuthor(i18n("Allen Winter"), QString(), QStringLiteral("winter@kde.org"));
    about.addAuthor(i18n("Rafael Fernández López"), QString(), QStringLiteral("ereslibre@kde.org"));
    about.addAuthor(i18n("Daniel Molkentin"), QString(), QStringLiteral("molkentin@kde.org"));
    about.addAuthor(i18n("Don Sanders"), QString(), QStringLiteral("sanders@kde.org"));
    about.addAuthor(i18n("Cornelius Schumacher"), QString(), QStringLiteral("schumacher@kde.org"));
    about.addAuthor(i18n("Tobias K\303\266nig"), QString(), QStringLiteral("tokoe@kde.org"));
    about.addAuthor(i18n("David Faure"), QString(), QStringLiteral("faure@kde.org"));
    about.addAuthor(i18n("Ingo Kl\303\266cker"), QString(), QStringLiteral("kloecker@kde.org"));
    about.addAuthor(i18n("Sven L\303\274ppken"), QString(), QStringLiteral("sven@kde.org"));
    about.addAuthor(i18n("Zack Rusin"), QString(), QStringLiteral("zack@kde.org"));
    about.addAuthor(i18n("Matthias Hoelzer-Kluepfel"),
                    i18n("Original Author"), QStringLiteral("mhk@kde.org"));
    about.addCredit(i18n("Torgny Nyblom"), i18n("Git Migration"), QStringLiteral("nyblom@kde.org"));
    about.setOrganizationDomain("kde.org");
    app.setAboutData(about);
    app.setWindowIcon(QIcon::fromTheme(QStringLiteral("kontact")));
    app.setDesktopFileName(QStringLiteral("org.kde.kontact"));

    QCommandLineParser *cmdArgs = app.cmdArgs();
    loadCommandLineOptions(cmdArgs);

    const QStringList args = QApplication::arguments();
    cmdArgs->process(args);
    about.processCommandLine(cmdArgs);

    if (cmdArgs->isSet(QStringLiteral("list"))) {
        listPlugins();
        return 0;
    }

    if (!KontactApp::start(args)) {
        // Already running, brought to the foreground.
        qCDebug(KONTACT_LOG) << "Kontact already running, exiting.";
        return 0;
    }

    if (!app.isSessionRestored()) {
        // There can only be one main window
        if (KMainWindow::canBeRestored(1)) {
            MainWindow *mainWindow = new MainWindow();
            app.setMainWindow(mainWindow);
            app.setSessionRestored(true);
            mainWindow->show();
            mainWindow->restore(1);
        }
    }

    const int ret = app.exec();
    qDeleteAll(KMainWindow::memberList());

    return ret;
}

#include "main.moc"
