/*
  This file is part of the KDE project

  Copyright (C) 2004 Michael Brade <brade@kde.org>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public
  License as published by the Free Software Foundation; either
  version 2 of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; see the file COPYING.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.

  In addition, as a special exception, the copyright holders give
  permission to link the code of this program with any edition of
  the Qt library by Trolltech AS, Norway (or with modified versions
  of Qt that use the same license as Qt), and distribute linked
  combinations including the two.  You must obey the GNU General
  Public License in all respects for all of the code used other than
  Qt.  If you modify this file, you may extend this exception to
  your version of the file, but you are not obligated to do so.  If
  you do not wish to do so, delete this exception statement from
  your version.
*/

#include "knotetip.h"
#include "knotes_part_p.h"

#include <KTextEdit>

#include <QAbstractEventDispatcher>
#include <QApplication>
#include <QBoxLayout>
#include <QToolTip>

KNoteTip::KNoteTip( QListWidget *parent )
  : QFrame( 0,
            Qt::Tool |
            Qt::X11BypassWindowManagerHint | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint ),
    mFilter( false ),
    mView( parent ),
    mNoteIVI( 0 ),
    mPreview( new KTextEdit( this ) )
{
  mPreview->setReadOnly( true );
  mPreview->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
  mPreview->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );

  QBoxLayout *layout = new QVBoxLayout( this );
  layout->addWidget( mPreview );
  layout->setMargin( 1 );
  setPalette( QToolTip::palette() );
  setFrameStyle( QFrame::Plain );
  hide();
}

KNoteTip::~KNoteTip()
{
  delete mPreview;
  mPreview = 0;
}

void KNoteTip::setNote( KNotesIconViewItem *item )
{
  if ( mNoteIVI == item ) {
    return;
  }

  mNoteIVI = item;
  if ( !mNoteIVI ) {
    QAbstractEventDispatcher::instance()->unregisterTimers(this);
    if ( isVisible() ) {
      setFilter( false );
      hide();
    }
  } else {
    Journal *journal = item->journal();
    mPreview->setAcceptRichText( journal->customProperty( "KNotes", "RichText" ) == QLatin1String("true") );

    const QColor fg( journal->customProperty( "KNotes", "FgColor" ) );
    const QColor bg( journal->customProperty( "KNotes", "BgColor" ) );
    setColor( fg, bg );

    mPreview->setText( journal->description() );
    //mPreview->zoomTo( 8 );
    //mPreview->sync(); this is deprecated in Qt4, but there is no replacement

    mPreview->document()->adjustSize ();
    int w = int( mPreview->document ()->size().width() );
    int h = int( mPreview->document ()->size().height() );
    while ( w > 60 && h == mPreview->heightForWidth( w - 20 ) ) {
      w -= 20;
    }

    QRect desk = KGlobalSettings::desktopGeometry( mView->visualItemRect( mNoteIVI ).center() );
    resize( w, qMin( h, desk.height() / 2 - 20 ) );

    hide();
    QAbstractEventDispatcher::instance()->unregisterTimers( this );
    setFilter( true );
    startTimer( 600 );  // delay showing the tooltip for 0.7 sec
  }
}

// protected, virtual methods

void KNoteTip::resizeEvent( QResizeEvent *ev )
{
  QFrame::resizeEvent( ev );
  reposition();
}

void KNoteTip::timerEvent( QTimerEvent * )
{
  QAbstractEventDispatcher::instance()->unregisterTimers( this );

  if ( !isVisible() ) {
    startTimer( 15000 ); // show the tooltip for 15 sec
    reposition();
    show();
  } else {
    setFilter( false );
    hide();
  }
}

bool KNoteTip::eventFilter( QObject *, QEvent *e )
{
  switch ( e->type() ) {
  case QEvent::Leave:
  case QEvent::MouseButtonPress:
  case QEvent::MouseButtonRelease:
  case QEvent::KeyPress:
  case QEvent::KeyRelease:
  case QEvent::FocusIn:
  case QEvent::FocusOut:
  case QEvent::Wheel:
    QAbstractEventDispatcher::instance()->unregisterTimers(this);
    setFilter( false );
    hide();
  default:
    break;
  }

  return false;
}

// private stuff

void KNoteTip::setColor( const QColor &fg, const QColor &bg )
{
  QPalette newpalette = palette();
  newpalette.setColor( QPalette::Background, bg );
  newpalette.setColor( QPalette::Foreground, fg );
  newpalette.setColor( QPalette::Base, bg ); // text background
  newpalette.setColor( QPalette::Text, fg ); // text color
  newpalette.setColor( QPalette::Button, bg );

  // the shadow
  newpalette.setColor( QPalette::Midlight, bg.light(110) );
  newpalette.setColor( QPalette::Shadow, bg.dark(116) );
  newpalette.setColor( QPalette::Light, bg.light(180) );
  newpalette.setColor( QPalette::Dark, bg.dark(108) );
  setPalette( newpalette );

  // set the text color
  mPreview->setTextColor( fg );
}

void KNoteTip::setFilter( bool enable )
{
  if ( enable == mFilter ) {
    return;
  }

  if ( enable ) {
    qApp->installEventFilter( this );
    setMouseTracking( true );
  } else {
    setMouseTracking( false );
    qApp->removeEventFilter( this );
  }

  mFilter = enable;
}

void KNoteTip::reposition()
{
  if ( !mNoteIVI ) {
    return;
  }

  QRect rect = mView->visualItemRect( mNoteIVI );
  QPoint off = mView->mapFromParent( mView->viewport()->mapToGlobal( QPoint( 0, 0 ) ) );
  rect.translate( off.x(), off.y() );

  QPoint pos = rect.center();
  // should the tooltip be shown to the left or to the right of the ivi?
  QRect desk = KGlobalSettings::desktopGeometry( pos );
  if ( rect.center().x() + width() > desk.right() ) {
    // to the left
    if ( pos.x() - width() < 0 ) {
      pos.setX( 0 );
    } else {
      pos.setX( pos.x() - width() );
    }
  }

  // should the tooltip be shown above or below the ivi ?
  if ( rect.bottom() + height() > desk.bottom() ) {
    // above
    pos.setY( rect.top() - height() );
  } else {
    pos.setY( rect.bottom() );
  }
  move( pos );
  update();
}
