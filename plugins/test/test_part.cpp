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

#include "test_part.h"
#include "kaddressbookiface_stub.h"

#include <qtextedit.h>
#include <qcombobox.h>

#include "sidebarextension.h"

#include <kmessagebox.h>
#include <klocale.h>
#include <kaction.h>
#include <kapplication.h>
#include <kdebug.h>
#include <dcopclient.h>
#include <kdcopservicestarter.h>
#include <ktrader.h>
#include <kservice.h>


TestPart::TestPart(QObject *parent, const char *name) // ## parentWidget
  : KParts::ReadOnlyPart(parent, name)
{
  setInstance( new KInstance("testpart") ); // ## memleak
  m_edit = new QTextEdit;
  setWidget(m_edit);
  setXMLFile("testpartui.rc");
  new KAction( "new contact (test)", 0, this, SLOT( newContact() ), actionCollection(), "test_deleteevent" );
  m_kab_stub = 0L;

  new KParts::SideBarExtension(new QComboBox(this), this, "sbe");

  kapp->dcopClient()->setNotifications( true );
  connect( kapp->dcopClient(), SIGNAL( applicationRemoved( const QCString&)),
           this, SLOT( unregisteredFromDCOP( const QCString& )) );
}

TestPart::~TestPart()
{
  kapp->dcopClient()->setNotifications( false );
  delete m_kab_stub;
}

void TestPart::newContact()
{
  if ( !connectToAddressBook() )
    return;

  kdDebug(5602) << "Calling newContact" << endl;
  m_kab_stub->newContact();

  // If critical call: test that it worked ok
  if ( !m_kab_stub->ok() ) {
    kdDebug(5602) << "Communication problem - ERROR" << endl;
    // TODO handle the error
  }
}

bool TestPart::connectToAddressBook()
{
  if ( !m_kab_stub )
  {
    QString error;
    QCString dcopService;
    int result = KDCOPServiceStarter::self()->findServiceFor( "DCOP/AddressBook", QString::null, QString::null, &error, &dcopService );
    if ( result != 0 ) {
      // You might want to show "error" (if not empty) here, using e.g. KMessageBox
      return false;
    }
    // TODO document the required named for the dcop interfaces e.g. "CalendarIface".
    QCString dcopObjectId = "KAddressBookIface";
    m_kab_stub = new KAddressBookIface_stub(kapp->dcopClient(), dcopService, dcopObjectId);
  }
  return m_kab_stub != 0L;
}

void TestPart::unregisteredFromDCOP( const QCString& appId )
{
  if ( m_kab_stub && m_kab_stub->app() == appId )
  {
    // Delete the stub so that the next time we need the addressbook,
    // we'll know that we need to start a new one.
    delete m_kab_stub;
    m_kab_stub = 0L;
  }
}

bool TestPart::openFile()
{
  m_edit->setText(m_file);
  return true;
}

#include "test_part.moc"
