/*
    This file is part of Kontact.
    Copyright (c) 2003 Tobias Koenig <tokoe@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef KCMKONTACTKNT_H
#define KCMKONTACTKNT_H

#include <kcmodule.h>
//Added by qt3to4:
#include <Q3ValueList>

class Q3ListViewItem;
class QSpinxBox;

class KAboutData;
class KListView;
class KPushButton;

class NewsItem;

class KCMKontactKNT : public KCModule
{
  Q_OBJECT

  public:
    KCMKontactKNT( KInstance * inst, QWidget *parent = 0);

    virtual void load();
    virtual void save();
    virtual void defaults();
    virtual const KAboutData* aboutData() const;

  private slots:
    void addNews();
    void removeNews();
    void newFeed();
    void deleteFeed();

    void selectedChanged( Q3ListViewItem *item );
    void allCurrentChanged( Q3ListViewItem *item );

    void modified();

  private:
    void initGUI();
    void loadNews();
    void loadCustomNews();
    void storeCustomNews();
    void scanNews();

    bool dcopActive() const;

    KListView *mAllNews;
    KListView *mSelectedNews;
    Q3ListViewItem *mCustomItem;

    KPushButton *mAddButton;
    KPushButton *mRemoveButton;
    KPushButton *mNewButton;
    KPushButton *mDeleteButton;
    QSpinBox *mUpdateInterval;
    QSpinBox *mArticleCount;

    QMap<QString, QString> mFeedMap;
    Q3ValueList<NewsItem*> mCustomFeeds;
};

class NewsEditDialog : public KDialogBase
{
  Q_OBJECT

  public:
    NewsEditDialog( const QString&, const QString&, QWidget *parent );
    QString title() const;
    QString url() const;

  private slots:
    void modified();

  private:
    QLineEdit *mTitle;
    QLineEdit *mURL;
};

#endif
