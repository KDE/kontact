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

QHash<Akonadi::Entity::Id, KNotesIconViewItem *> KNotesIconView::noteList() const
{
    return mNoteList;
}

KNotesIconViewItem::KNotesIconViewItem( const Akonadi::Item &item, QListWidget *parent )
    : QListWidgetItem( parent ),
      mItem(item),
      mDisplayAttribute(new KNoteDisplaySettings),
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
    updateSettings();
}

bool KNotesIconViewItem::readOnly() const
{
    return mReadOnly;
}

void KNotesIconViewItem::setReadOnly(bool b, bool save)
{
    mReadOnly = b;
    if (mItem.hasAttribute<NoteShared::NoteLockAttribute>()) {
        if (!mReadOnly) {
            mItem.removeAttribute<NoteShared::NoteLockAttribute>();
        }
    } else {
        if (mReadOnly) {
            mItem.attribute<NoteShared::NoteLockAttribute>( Akonadi::Entity::AddIfMissing );
        }
    }
    if (save) {
        Akonadi::ItemModifyJob *job = new Akonadi::ItemModifyJob(mItem);
        connect( job, SIGNAL(result(KJob*)), SLOT(slotNoteSaved(KJob*)) );
    }
}

void KNotesIconViewItem::setDisplayDefaultValue()
{
    KNoteUtils::setDefaultValue(mItem);
    Akonadi::ItemModifyJob *job = new Akonadi::ItemModifyJob(mItem);
    connect( job, SIGNAL(result(KJob*)), SLOT(slotNoteSaved(KJob*)) );
}

void KNotesIconViewItem::slotNoteSaved(KJob *job)
{
    qDebug()<<" void KNotesIconViewItem::slotNoteSaved(KJob *job)";
    if ( job->error() ) {
        qDebug()<<" problem during save note:"<<job->errorString();
    }
}


void KNotesIconViewItem::setIconText( const QString &text, bool save )
{
    QString replaceText ;
    if ( text.count() > 50 ) {
        replaceText = text.left(50) + QLatin1String("...");
    } else {
        replaceText = text ;
    }

    setText( replaceText );
    if (save)
        saveNoteContent(text);
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
    return QString::fromLatin1(noteMessage->mainBodyPart()->decodedContent());
}

void KNotesIconViewItem::setDescription(const QString &description)
{
    saveNoteContent(QString(), description);
}

KNoteDisplaySettings *KNotesIconViewItem::displayAttribute() const
{
    return mDisplayAttribute;
}

Akonadi::Item KNotesIconViewItem::item()
{
    return mItem;
}

void KNotesIconViewItem::saveNoteContent(const QString &subject, const QString &description)
{
    KMime::Message::Ptr message = mItem.payload<KMime::Message::Ptr>();
    const QByteArray encoding( "utf-8" );
    if (!subject.isEmpty()) {
        message->subject( true )->fromUnicodeString( subject, encoding );
    }
    message->contentType( true )->setMimeType( isRichText() ? "text/html" : "text/plain" );
    message->contentType()->setCharset(encoding);
    message->contentTransferEncoding(true)->setEncoding(KMime::Headers::CEquPr);
    message->date( true )->setDateTime( KDateTime::currentLocalDateTime() );
    if (!description.isEmpty()) {
        message->mainBodyPart()->fromUnicodeString( description );
    } else if (message->mainBodyPart()->decodedText().isEmpty()) {
        message->mainBodyPart()->fromUnicodeString( QString::fromLatin1( " " ) );
    }

    message->assemble();

    mItem.setPayload( message );
    Akonadi::ItemModifyJob *job = new Akonadi::ItemModifyJob(mItem);
    connect( job, SIGNAL(result(KJob*)), SLOT(slotNoteSaved(KJob*)) );
}


void KNotesIconViewItem::setChangeItem(const Akonadi::Item &item, const QSet<QByteArray> & set)
{
    mItem = item;
    if ( item.hasAttribute<NoteShared::NoteDisplayAttribute>()) {
        mDisplayAttribute->setDisplayAttribute(item.attribute<NoteShared::NoteDisplayAttribute>());
    }
    if (set.contains("ATR:KJotsLockAttribute")) {
        setReadOnly(item.hasAttribute<NoteShared::NoteLockAttribute>(), false);
    }
    if (set.contains("PLD:RFC822")) {
        KMime::Message::Ptr noteMessage = item.payload<KMime::Message::Ptr>();
        setIconText(noteMessage->subject(false)->asUnicodeString(), false);
    }
    if (set.contains("ATR:NoteDisplayAttribute")) {
        updateSettings();
    }

}
void KNotesIconViewItem::updateSettings()
{
    KIconEffect effect;
    const QColor color( mDisplayAttribute->backgroundColor() );
    QPixmap icon = KIconLoader::global()->loadIcon( QLatin1String("knotes"), KIconLoader::Desktop );
    icon = effect.apply( icon, KIconEffect::Colorize, 1, color, false );
    setFont(mDisplayAttribute->titleFont());
    setIcon( icon );
}

#include "moc_knotesiconview.cpp"
