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

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "kcmkontact.h"
#include "prefs.h"
#include <kcomponentdata.h>

#include <kaboutdata.h>
#include <kdebug.h>
#include <k3listview.h>
#include <klocale.h>
#include <kservicetypetrader.h>

#include <q3buttongroup.h>
#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLayout>
//Added by qt3to4:
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QBoxLayout>

#include <kdemacros.h>

extern "C"
{
  KDE_EXPORT KCModule *create_kontactconfig( QWidget *parent, const char * ) {
	KComponentData inst("kcmkontact" );
    return new KcmKontact( inst, parent );
  }
}

class PluginItem : public Q3ListViewItem
{
  public:
    PluginItem( Q3ListView *parent, const KService::Ptr &ptr )
      : Q3ListViewItem( parent, ptr->name(), ptr->comment(), ptr->library() ),
        mPtr( ptr )
    {
    }

    KService::Ptr servicePtr() const
    {
      return mPtr;
    }

  private:
    KService::Ptr mPtr;
};

KcmKontact::KcmKontact( const KComponentData &inst, QWidget *parent )
  : KPrefsModule( Kontact::Prefs::self(), inst, parent )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );
  QBoxLayout *pluginStartupLayout = new QHBoxLayout();
  topLayout->addItem( pluginStartupLayout );
  topLayout->addStretch();

  KPrefsWidBool *forceStartupPlugin = addWidBool( Kontact::Prefs::self()->forceStartupPluginItem(), this );
  pluginStartupLayout->addWidget( forceStartupPlugin->checkBox() );

  PluginSelection *selection = new PluginSelection( Kontact::Prefs::self()->forcedStartupPluginItem(), this );
  addWid( selection );

  pluginStartupLayout->addWidget( selection->comboBox() );
  selection->comboBox()->setEnabled( false );

  connect( forceStartupPlugin->checkBox(), SIGNAL( toggled( bool ) ),
           selection->comboBox(), SLOT( setEnabled( bool ) ) );
  load();
}

const KAboutData* KcmKontact::aboutData() const
{
  KAboutData *about = new KAboutData( I18N_NOOP( "kontactconfig" ), 0,
                                      ki18n( "KDE Kontact" ),
                                      0, KLocalizedString(), KAboutData::License_GPL,
                                      ki18n( "(c), 2003 Cornelius Schumacher" ) );

  about->addAuthor( ki18n("Cornelius Schumacher"), KLocalizedString(), "schumacher@kde.org" );
  about->addAuthor( ki18n("Tobias Koenig"), KLocalizedString(), "tokoe@kde.org" );

  return about;
}


PluginSelection::PluginSelection( KConfigSkeleton::ItemString *item, QWidget *parent )
{
  mItem = item;
  mPluginCombo = new QComboBox( parent );
  connect( mPluginCombo, SIGNAL( currentIndexChanged( int ) ), SIGNAL( changed() ) );
}

PluginSelection::~PluginSelection()
{
}

void PluginSelection::readConfig()
{
  const KService::List offers = KServiceTypeTrader::self()->query(
      QString::fromLatin1( "Kontact/Plugin" ),
      QString( "[X-KDE-KontactPluginVersion] == %1" ).arg( KONTACT_PLUGIN_VERSION ) );

  int activeComponent = 0;
  mPluginCombo->clear();
  for ( KService::List::ConstIterator it = offers.begin(); it != offers.end(); ++it ) {
    KService::Ptr service = *it;
    // skip summary only plugins
    QVariant var = service->property( "X-KDE-KontactPluginHasPart" );
    if ( var.isValid() && var.toBool() == false )
      continue;
    mPluginCombo->addItem( service->name() );
    mPluginList.append( service );

    if ( service->property("X-KDE-PluginInfo-Name").toString() == mItem->value() )
      activeComponent = mPluginList.count() - 1;
  }

  mPluginCombo->setCurrentIndex( activeComponent );
}

void PluginSelection::writeConfig()
{
  KService::Ptr ptr =  mPluginList.at( mPluginCombo->currentIndex() );
  mItem->setValue( ptr->property("X-KDE-PluginInfo-Name").toString() );
}

QList<QWidget *> PluginSelection::widgets() const
{
  QList<QWidget *> widgets;
  widgets.append( mPluginCombo );

  return widgets;
}

#include "kcmkontact.moc"
