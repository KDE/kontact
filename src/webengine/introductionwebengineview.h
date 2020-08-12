/*
   SPDX-FileCopyrightText: 2016-2020 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef INTRODUCTIONWEBENGINEVIEW_H
#define INTRODUCTIONWEBENGINEVIEW_H

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

#endif // INTRODUCTIONWEBENGINEVIEW_H
