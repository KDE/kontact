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
K_EXPORT_COMPONENT_FACTORY( libkpkaddressbookplugin,
                            KAddressbookPluginFactory( "kpaddressbookplugin" ) );

KAddressbookPlugin::KAddressbookPlugin( Kontact::Core *core, const char *name, const QStringList& )
  : Kontact::Plugin( core, core, name ), 
    mStub( 0 ),
    mPart( 0 )
{
  setInstance( KAddressbookPluginFactory::instance() );

  setXMLFile( "kpkaddressbookplugin.rc" );

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
                                                             0, 0, // parentwidget,name
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

KAboutData* KAddressbookPlugin::aboutData()
{
  KAboutData *about = new KAboutData( "kaddressbook", I18N_NOOP( "KAddressBook" ),
                                      "3.1", I18N_NOOP( "The KDE Address Book" ),
                                      KAboutData::License_GPL_V2,
                                      I18N_NOOP( "(c) 1997-2003, The KDE PIM Team" ) );
  about->addAuthor( "Tobias Koenig", I18N_NOOP( "Current maintainer" ), "tokoe@kde.org" );
  about->addAuthor( "Don Sanders", I18N_NOOP( "Original author" ) );
  about->addAuthor( "Cornelius Schumacher",
                    I18N_NOOP( "Co-maintainer, libkabc port, CSV import/export" ),
                    "schumacher@kde.org" );
  about->addAuthor( "Mike Pilone", I18N_NOOP( "GUI and framework redesign" ),
                    "mpilone@slac.com" );
  about->addAuthor( "Greg Stern", I18N_NOOP( "DCOP interface" ) );
  about->addAuthor( "Mark Westcott", I18N_NOOP( "Contact pinning" ) );
  about->addAuthor( "Mischel Boyer de la Giroday", I18N_NOOP( "LDAP Lookup" ),
                    "michel@klaralvdalens-datakonsult.se" );
  about->addAuthor( "Steffen Hansen", I18N_NOOP( "LDAP Lookup" ),
                    "hansen@kde.org" );
  return about;
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

QWidget *KAddressbookPlugin::createSummaryWidget( QWidget* parentWidget )
{
  return new KABSummaryWidget( parentWidget );
}

#include "kaddressbook_plugin.moc"
