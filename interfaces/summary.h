/*
   This file is part of KDE Kontact.

   Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/
#ifndef KONTACT_SUMMARY_H
#define KONTACT_SUMMARY_H

#include <qwidget.h>
#include <qpixmap.h>
#include <kdepimmacros.h>

class KStatusBar;

namespace Kontact
{

/**
  Summary widget for display in the Summary View plugin.
 */
class KDE_EXPORT Summary : public QWidget
{
  Q_OBJECT

  public:
    Summary( QWidget *parent, const char *name = 0 );

    virtual ~Summary();

    /**
      Return logical height of summary widget. This is used to calculate how
      much vertical space relative to other summary widgets this widget will use
      in the summary view.
    */
    virtual int summaryHeight() const { return 1; }

    /**
      Creates a heading for a typical summary view with an icon and a heading.
     */
    QWidget *createHeader( QWidget* parent, const QPixmap &icon,
                           const QString& heading );

    /**
      Return list of strings identifiying configuration modules for this summary
      part. The string has to be suitable for being passed to
      KCMultiDialog::addModule().
    */
    virtual QStringList configModules() const { return QStringList(); }

  public slots:
    virtual void configChanged() {}

    /**
      This is called if the displayed information should be updated.
      @param force true if the update was requested by the user
    */
    virtual void updateSummary( bool force = false ) { Q_UNUSED( force ); }

  signals:
    void message( const QString &message );
    void summaryWidgetDropped( QWidget *target, QWidget *widget, int alignment );

  protected:
    virtual void mousePressEvent( QMouseEvent* );
    virtual void mouseMoveEvent( QMouseEvent* );
    virtual void dragEnterEvent( QDragEnterEvent* );
    virtual void dropEvent( QDropEvent* );

  private:
    KStatusBar *mStatusBar;
    QPoint mDragStartPoint;

    class Private;
    Private *d;
};

}

#endif
