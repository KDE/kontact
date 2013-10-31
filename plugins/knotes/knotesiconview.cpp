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
#include "utils/knoteutils.h"
#include "knoteconfig.h"

#include <KCal/Journal>
using namespace KCal;

#include <KIconEffect>
#include <KIconLoader>
#include <QColor>
#include <QPixmap>
#include <QMouseEvent>

KNotesIconView::KNotesIconView( KNotesPart *part )
    : KListWidget(),
      m_part( part )
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
      mJournal( journal ),
      mConfig(0)
{
    QString configPath;

    mConfig = KNoteUtils::createConfig(journal, configPath);
    KNoteUtils::setProperty(journal, mConfig);

    updateSettings();
    setIconText( journal->summary() );
}

KNotesIconViewItem::~KNotesIconViewItem()
{
    delete mConfig;
}


void KNotesIconViewItem::updateSettings()
{
    KNoteUtils::savePreferences(mJournal, mConfig);
    KIconEffect effect;
    QColor color( mConfig->bgColor() );
    QPixmap icon = KIconLoader::global()->loadIcon( QLatin1String("knotes"), KIconLoader::Desktop );
    icon = effect.apply( icon, KIconEffect::Colorize, 1, color, false );
    setFont(mConfig->titleFont());
    mConfig->writeConfig();
    setIcon( icon );
}

Journal *KNotesIconViewItem::journal() const
{
    return mJournal;
}

KNoteConfig *KNotesIconViewItem::config()
{
    return mConfig;
}

QString KNotesIconViewItem::realName() const
{
    return mJournal->summary();
}

void KNotesIconViewItem::setIconText( const QString &text )
{
    QString replaceText ;
    if ( text.count() > 50 ) {
        replaceText = text.left(50) + QLatin1String("...");
    } else {
        replaceText = text ;
    }

    setText( replaceText );

    mJournal->setSummary( text );
}

bool KNotesIconViewItem::readOnly() const
{
    return mConfig->readOnly();
}

void KNotesIconViewItem::setReadOnly(bool b)
{
    mConfig->setReadOnly(b);
    mConfig->writeConfig();
}

int KNotesIconViewItem::tabSize() const
{
    return mConfig->tabSize();
}

bool KNotesIconViewItem::autoIndent() const
{
    return mConfig->autoIndent();
}

QFont KNotesIconViewItem::textFont() const
{
    return mConfig->font();
}
