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


#ifndef INTRODUCTIONWEBVIEW_H
#define INTRODUCTIONWEBVIEW_H

#include <KWebView>

class IntroductionWebView : public KWebView
{
    Q_OBJECT
public:
    explicit IntroductionWebView(QWidget *parent = Q_NULLPTR);
    ~IntroductionWebView();
Q_SIGNALS:
    void openUrl(const QUrl &url);
};

#endif // INTRODUCTIONWEBVIEW_H
