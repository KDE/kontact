/*
   SPDX-FileCopyrightText: 2016-2023 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "introductionwebenginepage.h"
#include <QFontDatabase>
#include <QFontInfo>
#include <QWebEngineProfile>
#include <QWebEngineSettings>
IntroductionWebEnginePage::IntroductionWebEnginePage(QObject *parent)
    : QWebEnginePage(parent)
{
    // Let's better be paranoid and disable plugins (it defaults to enabled):
    settings()->setAttribute(QWebEngineSettings::JavascriptEnabled, false);
    settings()->setAttribute(QWebEngineSettings::PluginsEnabled, false);
    settings()->setAttribute(QWebEngineSettings::JavascriptCanOpenWindows, false);
    settings()->setAttribute(QWebEngineSettings::JavascriptCanAccessClipboard, false);
    settings()->setAttribute(QWebEngineSettings::LocalStorageEnabled, false);
    settings()->setAttribute(QWebEngineSettings::XSSAuditingEnabled, false);
    settings()->setAttribute(QWebEngineSettings::ErrorPageEnabled, false);
    settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, false);
    // settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, false);

    settings()->setAttribute(QWebEngineSettings::ScreenCaptureEnabled, false);
    settings()->setAttribute(QWebEngineSettings::WebGLEnabled, false);
    settings()->setAttribute(QWebEngineSettings::AutoLoadIconsForPage, false);
    settings()->setAttribute(QWebEngineSettings::Accelerated2dCanvasEnabled, false);
    settings()->setAttribute(QWebEngineSettings::WebGLEnabled, false);

    settings()->setAttribute(QWebEngineSettings::FocusOnNavigationEnabled, false);
    settings()->setAttribute(QWebEngineSettings::AllowRunningInsecureContent, false);
    profile()->setPersistentCookiesPolicy(QWebEngineProfile::NoPersistentCookies);

    const QFontInfo font(QFontDatabase().systemFont(QFontDatabase::GeneralFont));
    settings()->setFontFamily(QWebEngineSettings::StandardFont, font.family());
    settings()->setFontSize(QWebEngineSettings::DefaultFontSize, font.pixelSize());
}

IntroductionWebEnginePage::~IntroductionWebEnginePage() = default;

bool IntroductionWebEnginePage::acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame)
{
    if (url.scheme() == QLatin1String("data")) {
        return true;
    }
    Q_UNUSED(type)
    Q_UNUSED(isMainFrame)
    Q_EMIT urlClicked(url);
    return false;
}
