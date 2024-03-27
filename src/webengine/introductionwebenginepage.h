/*
   SPDX-FileCopyrightText: 2016-2024 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QWebEnginePage>

class IntroductionWebEnginePage : public QWebEnginePage
{
    Q_OBJECT
public:
    explicit IntroductionWebEnginePage(QObject *parent = nullptr);
    ~IntroductionWebEnginePage() override;
Q_SIGNALS:
    void urlClicked(const QUrl &url);

protected:
    [[nodiscard]] bool acceptNavigationRequest(const QUrl &url, NavigationType type, bool isMainFrame) override;
};
