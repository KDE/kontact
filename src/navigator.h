/*
    This file is part of Kaplan
    Copyright (c) 2001 Matthias Hoelzer-Kluepfel <mhk@kde.org>
    Copyright (c) 2002 Daniel Molkentin <molkentin@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#ifndef NAVIGATOR_H
#define NAVIGATOR_H

#include <klistbox.h>
#include <qlist.h>

class QPixmap;
class QListBox;

/**
 * A @ref QListBoxPixmap Square Box with a large icon and a text
 * underneath.
 */
class EntryItem : public QListBoxPixmap // ### Abstract?
{
 public:
     EntryItem(QListBox *, const QPixmap&, const QString&, QObject*, const QCString&);
        ~EntryItem();
        /**
         * sets the icon for the item.
         * @param icon the icon to set
         * @param group the icongroup
         */
        void setIcon( const QString& icon, KIcon::Group group = KIcon::Panel );

        /* Does the virtal connection
         */
        void connectToReceiver( QObject * );

        /**
         * returns the width of this item.
         */
        virtual int width( const QListBox * ) const;
        /**
         * returns the height of this item.
         */
        virtual int height( const QListBox * ) const;

        /**
         * returns the pixmap.
         */
        virtual const QPixmap * pixmap() const {
            return m_Pixmap;
        }

    protected:
        virtual void paint( QPainter *p);
            
    private:
        QPixmap* m_Pixmap;
        QObject* m_receiver;
        QCString m_slot;
        QListBox* m_parent;
};

/**
 * Navigation pane showing all parts relevant to the user
 */
class Navigator : public KListBox
{
    Q_OBJECT

    public:

        Navigator(QWidget *parent=0, const char *name=0);

        void addEntry(QString text, QString icon, QObject *receiver, const char *slot);


        QSize sizeHint() const;

        private slots:

            void slotExecuted(QListBoxItem *item);


    private:
        /*
           QList<Entry> m_entries;

           int m_index;
         */

};

#endif
// vim: ts=4 sw=4 et
