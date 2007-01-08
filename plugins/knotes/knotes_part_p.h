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

#include <QLayout>
#include <QLabel>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QPixmap>
#include <QHBoxLayout>

#include <kactioncollection.h>
#include <klocale.h>
#include <k3iconview.h>
#include <kglobal.h>
#include <kiconloader.h>
#include <kiconeffect.h>
#include <klineedit.h>
#include <ktoolbar.h>
#include <kmenu.h>
#include <kdialog.h>
#include <kxmlguiclient.h>
#include <kxmlguifactory.h>
#include <kxmlguibuilder.h>

#include <kcal/journal.h>

#include "knotes/knoteedit.h"


class KNotesIconViewItem : public K3IconViewItem
{
  public:
    KNotesIconViewItem( K3IconView *parent, KCal::Journal *journal )
      : K3IconViewItem( parent ),
        mJournal( journal )
    {
      setRenameEnabled( true );

      KIconEffect effect;
      QColor color( journal->customProperty( "KNotes", "BgColor" ) );
      QPixmap icon = KIconLoader::global()->loadIcon( "knotes", K3Icon::Desktop );
      icon = effect.apply( icon, KIconEffect::Colorize, 1, color, false );
      setPixmap( icon );
      setText( journal->summary() );
    }

    KCal::Journal *journal()
    {
      return mJournal;
    }

    virtual void setText( const QString & text )
    {
      K3IconViewItem::setText( text );
      mJournal->setSummary( text );
    }

  private:
    KCal::Journal *mJournal;
};


class KNoteEditDlg : public KDialog, virtual public KXMLGUIClient
{
  Q_OBJECT

  public:
    KNoteEditDlg( QWidget *parent = 0, const char *name = 0 )
      : KDialog( parent)
    {
	  setCaption(i18n( "Edit Note" ));
	  setButtons(Ok | Cancel);
	  setDefaultButton(Ok);
	  setModal(true);
	  showButtonSeparator(true);
      // this dialog is modal to prevent one from editing the same note twice in two
      // different windows

      setInstance( new KInstance( "knotes" ) ); // TODO: hm, memleak??
      setXMLFile( "knotesui.rc" );
      actionCollection()->setAssociatedWidget( this );

      QWidget *page = new QWidget(this);
	  setMainWidget(page);
      QVBoxLayout *layout = new QVBoxLayout( page );

      QHBoxLayout *hbl = new QHBoxLayout();
      layout->addItem( hbl );
      hbl->setSpacing( marginHint() );
      QLabel *label = new QLabel( page );
      label->setText( i18n( "Name:" ) );
      hbl->addWidget( label,0 );
      mTitleEdit= new KLineEdit( page );
      mTitleEdit->setObjectName( "name" );
      hbl->addWidget( mTitleEdit, 1,Qt::AlignVCenter  );

      mNoteEdit = new KNoteEdit( actionCollection(), page );
      ((KTextEdit*)mNoteEdit)->setTextFormat( Qt::RichText );
      mNoteEdit->setFocus();

      KXMLGUIBuilder builder( page );
      KXMLGUIFactory factory( &builder, this );
      factory.addClient( this );

      mTool = static_cast<KToolBar *>(factory.container( "note_tool", this ));

      layout->addWidget( mTool );
      layout->addWidget( mNoteEdit );
    }

    QString text() const
    {
      return mNoteEdit->text();
    }

    void setText( const QString& text )
    {
      mNoteEdit->setText( text );
    }

    QString title() const
    {
      return mTitleEdit->text();
    }

    void setTitle( const QString& text )
    {
      mTitleEdit->setText( text );
    }

  private:
    KLineEdit *mTitleEdit;
    KNoteEdit  *mNoteEdit;
    KToolBar   *mTool;
    KMenu *mEditMenu;
};


#endif
