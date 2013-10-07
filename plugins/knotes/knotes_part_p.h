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

#ifndef KNOTES_PART_P_H
#define KNOTES_PART_P_H

#include "knotes/knoteedit.h"

#include <KCal/Journal>
using namespace KCal;

#include <KActionCollection>
#include <KComponentData>
#include <KDialog>
#include <KIconEffect>
#include <KIconLoader>
#include <KLineEdit>
#include <KListWidget>
#include <KLocale>
#include <KToolBar>
#include <KXMLGUIBuilder>
#include <KXMLGUIClient>
#include <KXMLGUIFactory>

#include <QAction>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QVBoxLayout>

class KNotesPart;

class KMenu;
class KTextEdit;

class KNotesIconView : public KListWidget
{
  public:
    explicit KNotesIconView( KNotesPart * );

  protected:
    void mousePressEvent( QMouseEvent * );

  private:
    KNotesPart *m_part;
};

class KNotesIconViewItem : public QListWidgetItem
{
  public:
    KNotesIconViewItem( QListWidget *parent, Journal *journal )
      : QListWidgetItem( parent ), mJournal( journal )
    {
      KIconEffect effect;
      QColor color( journal->customProperty( "KNotes", "BgColor" ) );
      QPixmap icon = KIconLoader::global()->loadIcon( QLatin1String("knotes"), KIconLoader::Desktop );
      icon = effect.apply( icon, KIconEffect::Colorize, 1, color, false );
      setIcon( icon );
      setIconText( journal->summary() );
    }

    Journal *journal()
    {
      return mJournal;
    }

    QString realName() const
    {
        return mJournal->summary();
    }

    void setIconText( const QString &text )
    {
      QString replaceText ;
      if ( text.count() > 5 ) {
        replaceText = text.left(5) + QLatin1String("...") ;
      } else {
        replaceText = text ;
      }

      setText( replaceText );

      mJournal->setSummary( text );
    }

  private:
    Journal *mJournal;
};

class KNoteEditDlg : public KDialog, virtual public KXMLGUIClient
{
  Q_OBJECT
  public:
    explicit KNoteEditDlg( QWidget *parent = 0 )
      : KDialog( parent )
    {
      setCaption( i18nc( "@title:window", "Edit Popup Note" ) );
      setButtons( Ok | Cancel );
      setDefaultButton( Ok );
      setModal( true );
      showButtonSeparator( true );
      // this dialog is modal to prevent one from editing the same note twice
      // in two different windows

      setComponentData( KComponentData( "knotes" ) ); // TODO: memleak
      setXMLFile( QLatin1String("knotesui.rc") );

      QWidget *page = new QWidget( this );
      setMainWidget( page );
      QVBoxLayout *layout = new QVBoxLayout( page );

      QHBoxLayout *hbl = new QHBoxLayout();
      layout->addItem( hbl );
      hbl->setSpacing( marginHint() );
      QLabel *label = new QLabel( page );
      label->setText( i18nc( "@label popup note name", "Name:" ) );
      hbl->addWidget( label, 0 );
      mTitleEdit= new KLineEdit( page );
      mTitleEdit->setObjectName( QLatin1String("name") );
      hbl->addWidget( mTitleEdit, 1, Qt::AlignVCenter );

      mNoteEdit = new KNoteEdit( actionCollection(), page );
      ( (KTextEdit*)mNoteEdit )->setAcceptRichText( true );
      mNoteEdit->setFocus();

      KXMLGUIBuilder builder( page );
      KXMLGUIFactory factory( &builder, this );
      factory.addClient( this );

      mTool = static_cast<KToolBar *>( factory.container( QLatin1String("note_tool"), this ) );
      layout->addWidget( mTool );
      layout->addWidget( mNoteEdit );

      actionCollection()->addAssociatedWidget( this );
      foreach ( QAction *action, actionCollection()->actions() ) {
        action->setShortcutContext( Qt::WidgetWithChildrenShortcut );
      }
    }

    QString text() const
    {
      return mNoteEdit->text();
    }

    void setText( const QString &text )
    {
      mNoteEdit->setText( text );
    }

    QString title() const
    {
      return mTitleEdit->text();
    }

    void setTitle( const QString &text )
    {
      mTitleEdit->setText( text );
    }
    KNoteEdit *noteEdit()
    {
      return mNoteEdit;
    }

  private:
    KLineEdit *mTitleEdit;
    KNoteEdit *mNoteEdit;
    KToolBar *mTool;
    KMenu *mEditMenu;
};

#endif
