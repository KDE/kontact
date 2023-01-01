/*
   SPDX-FileCopyrightText: 2016-2023 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <QWebEngineView>

class IntroductionWebEngineView : public QWebEngineView
{
    Q_OBJECT
public:
    explicit IntroductionWebEngineView(QWidget *parent = nullptr);
    ~IntroductionWebEngineView() override;

protected:
    void contextMenuEvent(QContextMenuEvent *ev) override;

Q_SIGNALS:
    void openUrl(const QUrl &url);
};
