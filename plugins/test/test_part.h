/*
    This file is part of KDE Kontact.

    Copyright (c) 2001 Matthias Hoelzer-Kluepfel <mhk@kde.org>

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
#ifndef __TEST_PART_H__
#define __TEST_PART_H__



#include <kparts/part.h>

class QTextEdit;
class KAddressBookIface_stub;

class TestPart : public KParts::ReadOnlyPart
{
  Q_OBJECT

public:

  TestPart(QObject *parent=0, const char *name=0);
    ~TestPart();

protected:

  virtual bool openFile();
  bool connectToAddressBook();

protected slots:

  void newContact();
  void unregisteredFromDCOP( const QCString& );

private:

  QTextEdit *m_edit;
  KAddressBookIface_stub *m_kab_stub;

};


#endif
