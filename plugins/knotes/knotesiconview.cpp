/*
  Copyright (c) 2013 Montel Laurent <montel@kde.org>

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

#include "knotesiconview.h"

#include <KCal/Journal>
using namespace KCal;

#include <KIconEffect>
#include <KIconLoader>
#include <QColor>
#include <QPixmap>
#include <QMouseEvent>

KNotesIconView::KNotesIconView( KNotesPart *part )
    : KListWidget(), m_part( part )
{
    setViewMode( QListView::IconMode );
    setMovement( QListView::Static );
    setSortingEnabled( true );
    setSelectionMode( QAbstractItemView::ExtendedSelection );
    setWordWrap( true );
    setMouseTracking ( true );
}

void KNotesIconView::mousePressEvent( QMouseEvent *e )
{
    if ( e->button() == Qt::RightButton ) {
        QListWidget::mousePressEvent( e );
        m_part->popupRMB( currentItem(), e->pos (), e->globalPos() );
    } else {
        KListWidget::mousePressEvent( e );
    }
}


KNotesIconViewItem::KNotesIconViewItem( QListWidget *parent, Journal *journal )
    : QListWidgetItem( parent ),
      mJournal( journal )
{
    KIconEffect effect;
    QColor color( journal->customProperty( "KNotes", "BgColor" ) );
    QPixmap icon = KIconLoader::global()->loadIcon( QLatin1String("knotes"), KIconLoader::Desktop );
    icon = effect.apply( icon, KIconEffect::Colorize, 1, color, false );
    setIcon( icon );
    setIconText( journal->summary() );
}

Journal *KNotesIconViewItem::journal() const
{
    return mJournal;
}

QString KNotesIconViewItem::realName() const
{
    return mJournal->summary();
}

void KNotesIconViewItem::setIconText( const QString &text )
{
    QString replaceText ;
    if ( text.count() > 50 ) {
        replaceText = text.left(50) + QLatin1String("...") ;
    } else {
        replaceText = text ;
    }

    setText( replaceText );

    mJournal->setSummary( text );
}
