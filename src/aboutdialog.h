/*
  This file is part of KDE Kontact.

  Copyright (c) 2003 Cornelius Schumacher <schumacher@kde.org>

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#ifndef KONTACT_ABOUTDIALOG_H
#define KONTACT_ABOUTDIALOG_H

#include <KPageDialog>

namespace KontactInterface {
class Core;
class Plugin;
}

class KAboutData;

namespace Kontact {

class AboutDialog : public KPageDialog
{
    Q_OBJECT

public:
    explicit AboutDialog( KontactInterface::Core *core );

protected:
    void addAboutPlugin( KontactInterface::Plugin *plugin );

    void addAboutData( const QString &title, const QString &icon,
                       const KAboutData *about );

    void addLicenseText( const KAboutData *about );

    QString formatPerson( const QString &name, const QString &email );

private slots:
    void saveSize();

private:
    KontactInterface::Core *mCore;
};

}

#endif
