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
   the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.

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

#include <qtooltip.h>
#include <qlayout.h>
#include <qtextedit.h>

#include <kapplication.h>
#include <kglobalsettings.h>

#include "knotetip.h"
#include "knotes_part_p.h"


KNoteTip::KNoteTip( KIconView *parent )
    : QFrame( 0, 0, WX11BypassWM |   // this will make Seli happy >:-P
              WStyle_Customize | WStyle_NoBorder | WStyle_Tool | WStyle_StaysOnTop ),
      m_filter( false ),
      m_view( parent ),
      m_noteIVI( 0 ),
      m_preview( new QTextEdit( this ) )
{
    m_preview->setReadOnly( true );
    m_preview->setHScrollBarMode( QScrollView::AlwaysOff );
    m_preview->setVScrollBarMode( QScrollView::AlwaysOff );

    QBoxLayout *layout = new QVBoxLayout( this );
    layout->addWidget( m_preview );

    setPalette( QToolTip::palette() );
    setMargin( 1 );
    setFrameStyle( QFrame::Plain | QFrame::Box );
    hide();
}

KNoteTip::~KNoteTip()
{
    delete m_preview;
}

void KNoteTip::setNote( KNotesIconViewItem *item, TextFormat format )
{
    if ( m_noteIVI == item && m_format == format )
        return;

    m_noteIVI = item;
    m_format = format;

    if ( !m_noteIVI )
    {
        killTimers();
        if ( isVisible() )
        {
            setFilter( false );
            hide();
        }
    }
    else
    {
        m_preview->setTextFormat( format );
        m_preview->setText( item->journal()->description() );
        m_preview->zoomTo( 6 );
        m_preview->sync();

        int w = 400;
        int h = m_preview->heightForWidth( w );
        while ( w > 60 && h == m_preview->heightForWidth( w - 20 ) )
            w -= 20;

        QRect desk = KGlobalSettings::desktopGeometry( m_noteIVI->rect().center() );
        resize( w, QMIN(h, desk.height()/2 - 20) );

        hide();
        killTimers();
        setFilter( true );
        startTimer( 700 );  // delay showing the tooltip for 0.7 sec
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
    killTimers();
    if ( !isVisible() )
    {
        startTimer( 15000 ); // show the tooltip for 15 sec
        reposition();
        show();
    }
    else
    {
        setFilter( false );
        hide();
    }
}

bool KNoteTip::eventFilter( QObject *, QEvent *e )
{
    switch ( e->type() )
    {
    case QEvent::Leave:
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    case QEvent::KeyPress:
    case QEvent::KeyRelease:
    case QEvent::FocusIn:
    case QEvent::FocusOut:
    case QEvent::Wheel:
        killTimers();
        setFilter( false );
        hide();
    default:
        break;
    }

    return false;
}


// private stuff

void KNoteTip::setFilter( bool enable )
{
    if ( enable == m_filter )
        return;

    if ( enable )
    {
        kapp->installEventFilter( this );
        QApplication::setGlobalMouseTracking( true );
    }
    else
    {
        QApplication::setGlobalMouseTracking( false );
        kapp->removeEventFilter( this );
    }

    m_filter = enable;
}

void KNoteTip::reposition()
{
    if ( !m_noteIVI )
        return;

    QRect rect = m_noteIVI->rect();
    QPoint off = m_view->mapToGlobal( m_view->contentsToViewport( QPoint( 0, 0 ) ) );
    rect.moveBy( off.x(), off.y() );

    QPoint pos = rect.center();

    // should the tooltip be shown to the left or to the right of the ivi?
    QRect desk = KGlobalSettings::desktopGeometry( pos );
    if ( rect.center().x() + width() > desk.right() )
    {
        // to the left
        if ( pos.x() - width() < 0 )
            pos.setX( 0 );
        else
            pos.setX( pos.x() - width() );
    }

    // should the tooltip be shown above or below the ivi ?
    if ( rect.bottom() + height() > desk.bottom() )
    {
        // above
        pos.setY( rect.top() - height() );
    }
    else
        pos.setY( rect.bottom() );

    move( pos );
    update();
}
