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

#ifndef KNOTESEDITDIALOG_H
#define KNOTESEDITDIALOG_H

#include <KDialog>
#include <KXMLGUIClient>
#include <KLineEdit>
#include "knotes/knoteedit.h"

class KNotesPart;
class KNoteEdit;
class KMenu;
class KTextEdit;
class KToolBar;
class KLineEdit;

class KNoteEditDialog : public KDialog, virtual public KXMLGUIClient
{
    Q_OBJECT
public:
    explicit KNoteEditDialog( QWidget *parent = 0 );

    void setAcceptRichText(bool b);
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

    KNoteEdit *noteEdit() const
    {
        return mNoteEdit;
    }

private:
    KLineEdit *mTitleEdit;
    KNoteEdit *mNoteEdit;
    KToolBar *mTool;
};

#endif // KNOTESEDITDIALOG_H
