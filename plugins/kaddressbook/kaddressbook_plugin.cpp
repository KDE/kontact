/*
    This file is part of Kontact.
    Copyright (c) 2003 Kontact Developer

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

#include <qwidget.h>

#include <kaboutdata.h>
#include <kaction.h>
#include <kapplication.h>
#include <kdebug.h>
#include <kgenericfactory.h>
#include <kiconloader.h>
#include <kmessagebox.h>
#include <kparts/componentfactory.h>

#include "core.h"
#include "kabsummarywidget.h"
#include "plugin.h"

#include "kaddressbook_plugin.h"

typedef KGenericFactory< KAddressbookPlugin, Kontact::Core > KAddressbookPluginFactory;
K_EXPORT_COMPONENT_FACTORY( libkontact_kaddressbookplugin,
                            KAddressbookPluginFactory( "kontact_kaddressbookplugin" ) )

KAddressbookPlugin::KAddressbookPlugin( Kontact::Core *core, const char *, const QStringList& )
  : Kontact::Plugin( core, core, "kaddressbook" ), 
    mStub( 0 ),
    mPart( 0 )
{
  setInstance( KAddressbookPluginFactory::instance() );

  insertNewAction( new KAction( i18n( "New Contact" ), BarIcon( "contact" ),
			             0, this, SLOT( slotNewContact() ), actionCollection(),
                   "new_contact" ) );
}

KAddressbookPlugin::~KAddressbookPlugin()
{
}

KParts::Part* KAddressbookPlugin::part()
{
  if ( !mPart ) {
    mPart = KParts::ComponentFactory
      ::createPartInstanceFromLibrary<KParts::ReadOnlyPart>( "libkaddressbookpart",
                                                             core(), 0, // parentwidget,name
                                                             this, 0 ); // parent,name
    if ( !mPart )
      return 0L;

    // 1) Register with dcop as "kaddressbook"  [maybe the part should do this]
    // 2) Create the stub that allows us to talk to the part
    mStub = new KAddressBookIface_stub( dcopClient(), "kaddressbook",
                                        "KAddressBookIface" );
  }

  return mPart;
}

QStringList KAddressbookPlugin::configModules() const
{
  QStringList modules;
  modules << "PIM/kabconfig.desktop" << "PIM/kabldapconfig.desktop";
  return modules;
}

void KAddressbookPlugin::slotNewContact()
{
  (void) part(); // ensure part is loaded
  Q_ASSERT( mStub );
  if ( mStub )
    mStub->newContact();
}

bool KAddressbookPlugin::createDCOPInterface( const QString& serviceType )
{
  if ( serviceType == "DCOP/AddressBook" )  {
    Q_ASSERT( mStub );
    return true;
  }

  return false;
}

Kontact::Summary *KAddressbookPlugin::createSummaryWidget( QWidget *parentWidget )
{
  return new KABSummaryWidget( this, parentWidget );
}

#include "kaddressbook_plugin.moc"
