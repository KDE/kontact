/*
   SPDX-FileCopyrightText: 2016-2020 Laurent Montel <montel@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "introductionwebengineview.h"
#include "introductionwebenginepage.h"

IntroductionWebEngineView::IntroductionWebEngineView(QWidget *parent)
    : QWebEngineView(parent)
{
    auto *pageEngine = new IntroductionWebEnginePage(this);
    setPage(pageEngine);

    setFocusPolicy(Qt::WheelFocus);
    connect(pageEngine, &IntroductionWebEnginePage::urlClicked, this, &IntroductionWebEngineView::openUrl);
}

IntroductionWebEngineView::~IntroductionWebEngineView()
{
}

void IntroductionWebEngineView::contextMenuEvent(QContextMenuEvent *)
{
    //Don't define contextmenu
}
