/*
    This file is part of KDE Kontact.

    Copyright (c) 2003 Cornelius Scumacher <schumacher@kde.org>

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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#ifndef KCMKONTACT_H
#define KCMKONTACT_H

#include <kprefsdialog.h>
#include <kservice.h>
#include "plugin.h"

class QGroupBox;
class QComboBOx;
class QListViewItem;

class KAboutData;
class KListView;

class KcmKontact : public KPrefsModule
{
  Q_OBJECT

  public:
    KcmKontact( QWidget *parent = 0, const char *name = 0 );

    virtual const KAboutData* aboutData() const;
};

class PluginSelection : public KPrefsWid
{
  Q_OBJECT

  public:
    PluginSelection( KConfigSkeleton::ItemString *item, QWidget *parent );
    ~PluginSelection();

    void readConfig();
    void writeConfig();

    QValueList<QWidget *> widgets() const;
    QComboBox *comboBox() const { return mPluginCombo; }

  private slots:
    void itemClicked( QListViewItem* );

  private:
    QComboBox *mPluginCombo;
    QValueList<KService::Ptr> mPluginList;
    KConfigSkeleton::ItemString *mItem;
};

#endif
