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

#ifndef SUMMARYWIDGET_H
#define SUMMARYWIDGET_H

#include <dcopobject.h>
#include <dcopref.h>

#include <qmap.h>
#include <qptrlist.h>
#include <qtimer.h>
#include <qwidget.h>

#include "summary.h"
#include <kurl.h>

class QVBoxLayout;
class QLabel;

class DCOPRef;
class KURLLabel;

typedef QValueList< QPair<QString, KURL> > ArticleMap;

typedef struct {
  DCOPRef ref;
  QString title;
  QString url;
  QPixmap logo;
  ArticleMap map;
} Feed;

typedef QValueList<Feed> FeedList;

class SummaryWidget : public Kontact::Summary, public DCOPObject
{
  Q_OBJECT
  K_DCOP

  public:
    SummaryWidget( QWidget *parent, const char *name = 0 );

    int summaryHeight() const;
    QStringList configModules() const;

  k_dcop:
    /**
     * Inform the newsticker summary widget that an RSSDocument has been updated.
     */
    void documentUpdated( DCOPRef );
    /**
     * Inform the newsticker summary widget that a feed has been added.
     */
    void documentAdded( QString );
    /**
     * Inform the newsticker summary widget that a feed has been removed.
     */
    void documentRemoved( QString );
    /**
     * Inform the newsticker summary widget that an error occurred while updating a feed.
     * @param ref DCOPRef to the failing RSSDocument.
     * @param errorCode indicates the cause of the failure: 1 = RSS Parse Error, 2 = Could not access file, 3 = Unknown error.
     */
    void documentUpdateError( DCOPRef ref, int errorCode );

  public slots:
    void updateSummary( bool force = false );
    void configChanged();

  protected slots:
    void updateDocuments();
    void rmbMenu( const QString& );

  protected:
    virtual bool eventFilter( QObject *obj, QEvent *e );
    void initDocuments();
    void updateView();
    void readConfig();

  private:
    QVBoxLayout *mLayout;
    QWidget* mBaseWidget;

    QPtrList<QLabel> mLabels;

    FeedList mFeeds;

    QTimer mTimer;
    int mUpdateInterval;
    int mArticleCount;
    uint mFeedCounter;
};

#endif
