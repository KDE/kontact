/*
  Copyright (c) 2016 Montel Laurent <montel@kde.org>

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

#include "introductionwebenginepage.h"
#include <QFontDatabase>
#include <QWebEngineSettings>
IntroductionWebEnginePage::IntroductionWebEnginePage(QObject *parent)
    : QWebEnginePage(parent)
{
    // Let's better be paranoid and disable plugins (it defaults to enabled):
    settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, false);
    //settings()->setAttribute(QWebEngineSettings::Javascript, false);
    //settings()->setAttribute(QWebEngineSettings::PluginsEnabled, false);

    const QFontInfo font(QFontDatabase().systemFont(QFontDatabase::GeneralFont));
    settings()->setFontFamily(QWebEngineSettings::StandardFont, font.family());
    settings()->setFontSize(QWebEngineSettings::DefaultFontSize, font.pixelSize());
}

IntroductionWebEnginePage::~IntroductionWebEnginePage()
{

}

bool IntroductionWebEnginePage::acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame)
{
    Q_UNUSED(type);
    Q_UNUSED(isMainFrame);
    Q_EMIT urlClicked(url);
    return false;
}
