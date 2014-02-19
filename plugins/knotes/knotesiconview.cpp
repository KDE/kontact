/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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
#include "noteshared/akonadi/notesakonaditreemodel.h"
#include "noteshared/attributes/notedisplayattribute.h"
#include "noteshared/attributes/notelockattribute.h"
#include "knotes/notes/knotedisplaysettings.h"
#include "utils/knoteutils.h"

#include <KLocalizedString>

#include <Akonadi/ItemModifyJob>

#include <KMime/KMimeMessage>

#include <KIconEffect>
#include <KIconLoader>

#include <QColor>
#include <QPixmap>
#include <QMouseEvent>
#include <QListWidgetItem>
#include <QDebug>

KNotesIconView::KNotesIconView( KNotesPart *part, QWidget *parent )
    : KListWidget(parent),
      m_part(part)
{
    setViewMode( QListView::IconMode );
    setMovement( QListView::Static );
    //setSortingEnabled( true );
    setSelectionMode( QAbstractItemView::ExtendedSelection );
    setWordWrap( true );
    setMouseTracking ( true );
}

KNotesIconView::~KNotesIconView()
{

}

void KNotesIconView::mousePressEvent( QMouseEvent *e )
{
    if ( e->button() == Qt::RightButton ) {
        QListView::mousePressEvent( e );
        m_part->popupRMB( currentItem(), e->pos (), e->globalPos() );
    } else {
        QListView::mousePressEvent( e );
    }
}

void KNotesIconView::addNote(const Akonadi::Item &item)
{
    KNotesIconViewItem *iconView = new KNotesIconViewItem(item, this);
    mNoteList.insert(item.id(), iconView);
}

KNotesIconViewItem *KNotesIconView::iconView(Akonadi::Item::Id id) const
{
    if (mNoteList.contains(id)) {
        return mNoteList.value(id);
    }
    return 0;
}

KNotesIconViewItem::KNotesIconViewItem( const Akonadi::Item &item, QListWidget *parent )
    : QListWidgetItem( parent ),
      mDisplayAttribute(new KNoteDisplaySettings),
      mItem(item),
      mReadOnly(false)
{
    if ( mItem.hasAttribute<NoteShared::NoteDisplayAttribute>()) {
        mDisplayAttribute->setDisplayAttribute(mItem.attribute<NoteShared::NoteDisplayAttribute>());
    } else {
        setDisplayDefaultValue();
        //save default display value
    }
    prepare();
}

KNotesIconViewItem::~KNotesIconViewItem()
{
    delete mDisplayAttribute;
}

void KNotesIconViewItem::prepare()
{
    KMime::Message::Ptr noteMessage = mItem.payload<KMime::Message::Ptr>();
    setText(noteMessage->subject(false)->asUnicodeString());

    if ( mItem.hasAttribute<NoteShared::NoteLockAttribute>() ) {
        mReadOnly = true;
    } else {
        mReadOnly = false;
    }
    //TODO slotUpdateReadOnly();
    // HACK: update the icon color - again after showing the note, to make kicker
    // aware of the new colors

    KIconEffect effect;
    QColor color( mDisplayAttribute->backgroundColor() );
    QPixmap icon = KIconLoader::global()->loadIcon( QLatin1String("knotes"), KIconLoader::Desktop );
    icon = effect.apply( icon, KIconEffect::Colorize, 1, color, false );
    setFont(mDisplayAttribute->titleFont());
    setIcon( icon );
}

bool KNotesIconViewItem::readOnly() const
{
    return mReadOnly;
}

void KNotesIconViewItem::setReadOnly(bool b)
{
    mReadOnly = b;
    //TODO update it.
}


void KNotesIconViewItem::setDisplayDefaultValue()
{
    KNoteUtils::setDefaultValue(mItem);
    Akonadi::ItemModifyJob *job = new Akonadi::ItemModifyJob(mItem);
    connect( job, SIGNAL(result(KJob*)), SLOT(slotNoteSaved(KJob*)) );
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

    //TODO
    //mJournal->setSummary( text );
}

QString KNotesIconViewItem::realName() const
{
    const KMime::Message::Ptr noteMessage = mItem.payload<KMime::Message::Ptr>();
    return noteMessage->subject(false)->asUnicodeString();
}

int KNotesIconViewItem::tabSize() const
{
    return mDisplayAttribute->tabSize();
}

bool KNotesIconViewItem::autoIndent() const
{
    return mDisplayAttribute->autoIndent();
}

QFont KNotesIconViewItem::textFont() const
{
    return mDisplayAttribute->font();
}

bool KNotesIconViewItem::isRichText() const
{
    const KMime::Message::Ptr noteMessage = mItem.payload<KMime::Message::Ptr>();
    return noteMessage->contentType()->isHTMLText();
}

QString KNotesIconViewItem::description() const
{
    const KMime::Message::Ptr noteMessage = mItem.payload<KMime::Message::Ptr>();
    return QString(); //TODO
}

KNoteDisplaySettings *KNotesIconViewItem::displayAttribute() const
{
    return mDisplayAttribute;
}



#if 0
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



#endif
#include "moc_knotesiconview.cpp"
