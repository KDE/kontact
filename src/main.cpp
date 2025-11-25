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

#include <QCommandLineParser>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QPointer>
#include <QWindow>

#include <iostream>

#include <KIconTheme>

#include <KStyleManager>

using namespace std;
using namespace Qt::Literals::StringLiterals;

static const char version[] = KONTACT_VERSION;

class KontactApp : public KontactInterface::PimUniqueApplication
{
    Q_OBJECT
public:
    KontactApp(int &argc, char **argv[])
        : KontactInterface::PimUniqueApplication(argc, argv)
        , mMainWindow(nullptr)
    {
        KLocalizedString::setApplicationDomain(QByteArrayLiteral("kontact"));
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
    const QList<KPluginMetaData> pluginMetaDatas = KPluginMetaData::findPlugins(u"pim6/kontact"_s, [](const KPluginMetaData &data) {
        return data.rawData().value(u"X-KDE-KontactPluginVersion"_s).toInt() == KONTACT_PLUGIN_VERSION;
    });

    for (const KPluginMetaData &plugin : pluginMetaDatas) {
        // skip summary only plugins
        if (plugin.rawData().contains("X-KDE-KontactPluginHasPart"_L1)) {
            bool var = plugin.rawData().value(u"X-KDE-KontactPluginHasPart"_s).toBool();

            if (!var) {
                continue;
            }
        }

        cout << "Plugin name " << qPrintable(plugin.pluginId()) << endl;
    }
}

static void loadCommandLineOptions(QCommandLineParser *parser)
{
    parser->addOption(QCommandLineOption(u"module"_s, i18nc("@info:shell", "Start with a specific Kontact module"), i18n("Module")));
    parser->addOption(QCommandLineOption(u"iconify"_s, i18nc("@info:shell", "Start in iconified (minimized) mode")));
    parser->addOption(QCommandLineOption(u"list"_s, i18nc("@info:shell", "List all possible modules and exit")));
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
    if (parser.isSet(u"module"_s)) {
        moduleName = parser.value(u"module"_s);
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
            if (parser.isSet(u"iconify"_s)) {
                mMainWindow->windowHandle()->showMinimized();
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
    QDBusConnection::sessionBus().interface()->startService(u"org.kde.kalendarac"_s);

    // Handle startup notification and window activation
    // (The first time it will do nothing except note that it was called)
    return 0;
}

int main(int argc, char **argv)
{
    KIconTheme::initTheme();
    QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    KontactApp app(argc, &argv);

    KStyleManager::initStyle();
    KAboutData about(u"kontact"_s,
                     i18n("Kontact"),
                     QLatin1StringView(version),
                     i18n("KDE personal information manager"),
                     KAboutLicense::GPL,
                     i18n("Copyright © 2001–%1 Kontact authors", u"2025"_s),
                     QString(),
                     u"https://userbase.kde.org/Kontact"_s);

    about.addAuthor(i18nc("@info:credit", "Allen Winter"), QString(), u"winter@kde.org"_s);
    about.addAuthor(i18nc("@info:credit", "Rafael Fernández López"), QString(), u"ereslibre@kde.org"_s);
    about.addAuthor(i18nc("@info:credit", "Daniel Molkentin"), QString(), u"molkentin@kde.org"_s);
    about.addAuthor(i18nc("@info:credit", "Don Sanders"), QString(), u"sanders@kde.org"_s);
    about.addAuthor(i18nc("@info:credit", "Cornelius Schumacher"), QString(), u"schumacher@kde.org"_s);
    about.addAuthor(i18nc("@info:credit", "Tobias K\303\266nig"), QString(), u"tokoe@kde.org"_s);
    about.addAuthor(i18nc("@info:credit", "David Faure"), QString(), u"faure@kde.org"_s);
    about.addAuthor(i18nc("@info:credit", "Ingo Kl\303\266cker"), QString(), u"kloecker@kde.org"_s);
    about.addAuthor(i18nc("@info:credit", "Sven L\303\274ppken"), QString(), u"sven@kde.org"_s);
    about.addAuthor(i18nc("@info:credit", "Zack Rusin"), QString(), u"zack@kde.org"_s);
    about.addAuthor(i18nc("@info:credit", "Matthias Hoelzer-Kluepfel"), i18n("Original Author"), u"mhk@kde.org"_s);
    about.addCredit(i18nc("@info:credit", "Torgny Nyblom"), i18n("Git Migration"), u"nyblom@kde.org"_s);
    app.setAboutData(about);
    app.setWindowIcon(QIcon::fromTheme(u"kontact"_s));
    app.setDesktopFileName(u"org.kde.kontact"_s);

    QApplication::setWindowIcon(QIcon::fromTheme(u"kontact"_s));
    KCrash::initialize();

    QCommandLineParser *cmdArgs = app.cmdArgs();
    loadCommandLineOptions(cmdArgs);

    const QStringList args = QApplication::arguments();
    cmdArgs->process(args);
    about.processCommandLine(cmdArgs);

    if (cmdArgs->isSet(u"list"_s)) {
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
