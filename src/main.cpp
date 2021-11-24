/*
  This file is part of KDE Kontact.

  SPDX-FileCopyrightText: 2001 Matthias Hoelzer-Kluepfel <mhk@kde.org>
  SPDX-FileCopyrightText: 2002-2003 Daniel Molkentin <molkentin@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

// Use the kdepim version
#include "kontact-version.h"

#include "kontact_debug.h"
#include "mainwindow.h"
#include "prefs.h"
using namespace Kontact;

#include <KontactInterface/PimUniqueApplication>
#include <KontactInterface/Plugin>
#include <KontactInterface/UniqueAppHandler>

#include <KAboutData>
#include <KCrash>
#include <KLocalizedString>
#include <KPluginMetaData>
#include <KWindowSystem>

#include <QCommandLineParser>
#include <QDBusConnection>
#include <QDBusConnectionInterface>

#include <iostream>
#include <kcoreaddons_version.h>
#if KCOREADDONS_VERSION < QT_VERSION_CHECK(6, 0, 0)
#include <kdelibs4configmigrator.h>
#endif
using namespace std;

static const char version[] = KONTACT_VERSION;

class KontactApp : public KontactInterface::PimUniqueApplication
{
    Q_OBJECT
public:
    KontactApp(int &argc, char **argv[])
        : KontactInterface::PimUniqueApplication(argc, argv)
        , mMainWindow(nullptr)
    {
        KLocalizedString::setApplicationDomain("kontact");
    }

    ~KontactApp() override = default;

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
    bool mSessionRestored = false;
};

static void listPlugins()
{
    const QVector<KPluginMetaData> pluginMetaDatas = KPluginMetaData::findPlugins(QStringLiteral("kontact5"), [](const KPluginMetaData &data) {
        return data.rawData().value(QStringLiteral("X-KDE-KontactPluginVersion")).toInt() == KONTACT_PLUGIN_VERSION;
    });

    for (const KPluginMetaData &plugin : pluginMetaDatas) {
        // skip summary only plugins
        if (plugin.rawData().contains(QLatin1String("X-KDE-KontactPluginHasPart"))) {
            bool var = plugin.rawData().value(QStringLiteral("X-KDE-KontactPluginHasPart")).toBool();

            if (!var) {
                continue;
            }
        }

        cout << "lib name " << qPrintable(plugin.pluginId().remove(QStringLiteral("kontact_"))) << endl;
    }
}

static void loadCommandLineOptions(QCommandLineParser *parser)
{
    parser->addOption(QCommandLineOption(QStringLiteral("module"), i18n("Start with a specific Kontact module"), QStringLiteral("module")));
    parser->addOption(QCommandLineOption(QStringLiteral("iconify"), i18n("Start in iconified (minimized) mode")));
    parser->addOption(QCommandLineOption(QStringLiteral("list"), i18n("List all possible modules and exit")));
}

int KontactApp::activate(const QStringList &args, const QString &workingDir)
{
    Q_UNUSED(workingDir)

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

    // Start KOrgac in case it's wasn't started on session start.
    QDBusConnection::sessionBus().interface()->startService(QStringLiteral("org.kde.korgac"));

    // Handle startup notification and window activation
    // (The first time it will do nothing except note that it was called)
    return 0;
}

int main(int argc, char **argv)
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps, true);
    KontactApp app(argc, &argv);

    KCrash::initialize();
#if KCOREADDONS_VERSION < QT_VERSION_CHECK(6, 0, 0)
    Kdelibs4ConfigMigrator migrate(QStringLiteral("kontact"));
    migrate.setConfigFiles(QStringList() << QStringLiteral("kontactrc") << QStringLiteral("kontact_summaryrc"));
    migrate.setUiFiles(QStringList() << QStringLiteral("kontactui.rc"));
    migrate.migrate();
#endif
    KAboutData about(QStringLiteral("kontact"),
                     i18n("Kontact"),
                     QLatin1String(version),
                     i18n("KDE personal information manager"),
                     KAboutLicense::GPL,
                     i18n("Copyright © 2001–2021 Kontact authors"),
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
    about.addAuthor(i18n("Matthias Hoelzer-Kluepfel"), i18n("Original Author"), QStringLiteral("mhk@kde.org"));
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
            auto mainWindow = new MainWindow();
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
