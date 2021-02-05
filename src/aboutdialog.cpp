/*
  This file is part of KDE Kontact.

  SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later WITH Qt-Commercial-exception-1.0
*/

#include "aboutdialog.h"
using namespace Kontact;

#include <KontactInterface/Core>
#include <KontactInterface/Plugin>

#include <KAboutData>
#include <KConfigGroup>
#include <KIconLoader>
#include <KLocalizedString>
#include <QIcon>
#include <QTextBrowser>

#include <KSharedConfig>
#include <KWindowConfig>
#include <QBoxLayout>
#include <QLabel>
#include <QPushButton>

AboutDialog::AboutDialog(KontactInterface::Core *core)
    : KPageDialog(core)
    , mCore(core)
{
    setWindowTitle(i18nc("@title:window", "About Kontact"));
    setStandardButtons(QDialogButtonBox::Close);
    button(QDialogButtonBox::Close)->setDefault(true);
    setModal(false);
    setFaceType(KPageDialog::List);
    addAboutData(i18n("Kontact Container"), QStringLiteral("kontact"), KAboutData::applicationData());
    QList<KontactInterface::Plugin *> plugins = mCore->pluginList();
    const QList<KontactInterface::Plugin *>::ConstIterator end = plugins.constEnd();
    QList<KontactInterface::Plugin *>::ConstIterator it = plugins.constBegin();
    for (; it != end; ++it) {
        addAboutPlugin(*it);
    }
    addLicenseText(KAboutData::applicationData());
    resize(QSize(600, 400));
    KConfigGroup grp = KConfigGroup(KSharedConfig::openConfig(), "AboutDialog");
    KWindowConfig::restoreWindowSize(windowHandle(), grp);
    connect(this, &AboutDialog::finished, this, &AboutDialog::saveSize);
}

void AboutDialog::saveSize()
{
    KConfigGroup group(KSharedConfig::openConfig(), "AboutDialog");
    KWindowConfig::saveWindowSize(windowHandle(), group);
    group.sync();
}

void AboutDialog::addAboutPlugin(KontactInterface::Plugin *plugin)
{
    addAboutData(plugin->title(), plugin->icon(), plugin->aboutData());
}

void AboutDialog::addAboutData(const QString &title, const QString &icon, const KAboutData &about)
{
    QIcon pixmap = QIcon::fromTheme(icon);

    QFrame *topFrame = new QFrame();
    auto pageItem = new KPageWidgetItem(topFrame, title);
    pageItem->setIcon(pixmap);

    addPage(pageItem);

    QBoxLayout *topLayout = new QVBoxLayout(topFrame);

    if (about.displayName().isEmpty()) {
        QLabel *label = new QLabel(i18n("No about information available."), topFrame);
        topLayout->addWidget(label);
    } else {
        QString text = QStringLiteral("<p>");
        text += QLatin1String("<b>") + about.displayName() + QLatin1String("</b>");
        text += QLatin1String("<br>");

        text += i18n("Version %1", about.version());
        text += QLatin1String("</p>");

        if (!about.shortDescription().isEmpty()) {
            text += QLatin1String("<p>") + about.shortDescription() + QLatin1String("<br>") + about.copyrightStatement() + QLatin1String("</p>");
        }

        QString home = about.homepage();
        if (!home.isEmpty()) {
            text += QLatin1String("<a href=\"") + home + QLatin1String("\">") + home + QLatin1String("</a><br>");
        }

        text.replace(QLatin1Char('\n'), QStringLiteral("<br>"));

        QLabel *label = new QLabel(text, topFrame);
        label->setAlignment(Qt::AlignTop);
        label->setOpenExternalLinks(true);
        label->setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard | Qt::LinksAccessibleByMouse);
        topLayout->addWidget(label);

        auto personView = new QTextBrowser(topFrame);
        personView->setReadOnly(true);
        topLayout->addWidget(personView, 1);

        text.clear();
        const QList<KAboutPerson> authors = about.authors();
        if (!authors.isEmpty()) {
            text += i18n("<p><b>Authors:</b></p>");
            const QList<KAboutPerson>::ConstIterator end(authors.end());
            for (QList<KAboutPerson>::ConstIterator it = authors.begin(); it != end; ++it) {
                text += formatPerson((*it).name(), (*it).emailAddress());
                if (!(*it).task().isEmpty()) {
                    text += QLatin1String("<i>") + (*it).task() + QLatin1String("</i><br>");
                }
            }
        }

        const QList<KAboutPerson> credits = about.credits();
        if (!credits.isEmpty()) {
            text += i18n("<p><b>Thanks to:</b></p>");

            const QList<KAboutPerson>::ConstIterator end(credits.end());
            for (QList<KAboutPerson>::ConstIterator it = credits.begin(); it != end; ++it) {
                text += formatPerson((*it).name(), (*it).emailAddress());
                if (!(*it).task().isEmpty()) {
                    text += QLatin1String("<i>") + (*it).task() + QLatin1String("</i><br>");
                }
            }
        }

        const QList<KAboutPerson> translators = about.translators();
        if (!translators.isEmpty()) {
            text += i18n("<p><b>Translators:</b></p>");
            const QList<KAboutPerson>::ConstIterator end(translators.end());
            for (QList<KAboutPerson>::ConstIterator it = translators.begin(); it != end; ++it) {
                text += formatPerson((*it).name(), (*it).emailAddress());
            }
        }
        // krazy:excludeall=style (really need krazy conditional code sections)
        text += i18n(
            "<br /><br />\
                     <i>This Free Software product was improved as part of a commercial project:</i>\
                     <h3>Credits</h3>\
                     Project Kowi (March 2007 - )<br /><br />\
                     \
                     <b>Production / Project Coordination</b><br />\
                     Bernhard Reiter<br />\
                     <b>Director of Development</b><br />\
                     Till Adam\
                     <br /><br />\
                     \
                     <table border=\"0\" width=\"100%\"> \
                <tr>\
                <td colspan=\"2\">\
                <b>Osnabr&uuml;ck Unit, Intevation GmbH</b>\
                </td>\
                </tr>\
                <tr>\
                <td width=\"60%\">\
                Unit Lead<br />\
                Senior QA, Packaging<br />\
                QA, Packaging<br />\
                Additional NSIS<br />\
                Backup Coordinator\
                </td>\
                <td>\
                Bernhard Reiter<br />\
                Bernhard Herzog<br />\
                Ludwig Reiter<br />\
                Emanuel Sch&uuml;tze<br />\
                Dr. Jan-Oliver Wagner\
                </td>\
                </tr>\
                </table>\
                <br /><br />\
                \
                <table border=\"0\" width=\"100%\">\
                <tr>\
                <td colspan=\"2\">\
                <b>Berlin Unit, Klar&auml;lvdalens Datakonsult AB</b>\
                </td>\
                </tr>\
                <tr>\
                <td width=\"60%\">\
                Unit Lead<br />\
                Development<br />\
                Development<br />\
                Additional D. + Crypto<br />\
                Crypto GUI Development\
                </td>\
                <td>\
                Till Adam<br />\
                Jaroslaw Staniek<br />\
                Volker Krause<br />\
                Frank Osterfeld<br />\
                Marc Mutz\
                </td>\
                </tr>\
                </table>\
                <br /><br />\
                \
                <table border=\"0\" width=\"100%\">\
                <tr>\
                <td colspan=\"2\">\
                <b>D&uuml;sseldorf Unit, g10 Code GmbH</b>\
                </td>\
                </tr>\
                <tr>\
                <td width=\"60%\">\
                Crypto-Backend Porting<br />\
                Crypto-Backend\
                </td>\
                <td>\
                Werner Koch<br />\
                Marcus Brinkmann\
                </td>\
                </tr>\
                </table>\
                <br /><br />\
                \
                <table border=\"0\" width=\"100%\">\
                <tr>\
                <td colspan=\"2\">\
                <b>External QA Darmstadt, basysKom GmbH</b>\
                </td>\
                </tr>\
                <tr>\
                <td width=\"60%\">\
                Unit-Lead\
                </td>\
                <td>\
                Dr. Stefan Werden\
                </td>\
                </tr>\
                </table><br /><br />");

        personView->setText(text);
    }
}

QString AboutDialog::formatPerson(const QString &name, const QString &email)
{
    QString text = name;
    if (!email.isEmpty()) {
        text += QLatin1String(" &lt;<a href=\"mailto:") + email + QLatin1String("\">") + email + QLatin1String("</a>&gt;");
    }

    text += QLatin1String("<br>");
    return text;
}

void AboutDialog::addLicenseText(const KAboutData &about)
{
    if (about.licenses().isEmpty()) {
        return;
    }
    QPixmap pixmap = KIconLoader::global()->loadIcon(QStringLiteral("help-about"), KIconLoader::Desktop, 48);

    const QString title = i18n("%1 License", about.displayName());

    QFrame *topFrame = new QFrame();
    auto page = new KPageWidgetItem(topFrame, title);
    page->setIcon(QIcon(pixmap));
    addPage(page);
    QBoxLayout *topLayout = new QVBoxLayout(topFrame);

    auto textBrowser = new QTextBrowser(topFrame);
    QString licenseStr;
    const QList<KAboutLicense> lstLicenses = about.licenses();
    for (const KAboutLicense &license : lstLicenses) {
        licenseStr += QStringLiteral("<pre>%1</pre>").arg(license.text());
    }
    textBrowser->setHtml(licenseStr);

    topLayout->addWidget(textBrowser);
}
