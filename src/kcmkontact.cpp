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
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    As a special exception, permission is given to link this program
    with any edition of Qt, and distribute the resulting executable,
    without including the source code for Qt in the source distribution.
*/

#include "kcmkontact.h"
#include "prefs.h"


#include <kaboutdata.h>
#include <kdebug.h>
#include <klistview.h>
#include <klocale.h>
#include <ktrader.h>

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qcombobox.h>
#include <qlabel.h>
#include <qlayout.h>

#include <kdepimmacros.h>

extern "C"
{
  KDE_EXPORT KCModule *create_kontactconfig( QWidget *parent, const char * ) {
    return new KcmKontact( parent, "kcmkontact" );
  }
}

class PluginItem : public QListViewItem
{
  public:
    PluginItem( QListView *parent, const KService::Ptr &ptr )
      : QListViewItem( parent, ptr->name(), ptr->comment(), ptr->library() ),
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

KcmKontact::KcmKontact( QWidget *parent, const char *name )
  : KPrefsModule( Kontact::Prefs::self(), parent, name )
{
  QBoxLayout *topLayout = new QVBoxLayout( this );
  QBoxLayout *pluginStartupLayout = new QHBoxLayout( topLayout );
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
  KAboutData *about = new KAboutData( I18N_NOOP( "kontactconfig" ),
                                      I18N_NOOP( "KDE Kontact" ),
                                      0, 0, KAboutData::License_GPL,
                                      I18N_NOOP( "(c), 2003 Cornelius Schumacher" ) );

  about->addAuthor( "Cornelius Schumacher", 0, "schumacher@kde.org" );
  about->addAuthor( "Tobias Koenig", 0, "tokoe@kde.org" );

  return about;
}


PluginSelection::PluginSelection( KConfigSkeleton::ItemString *item, QWidget *parent )
{
  mItem = item;
  mPluginCombo = new QComboBox( parent );
}

PluginSelection::~PluginSelection()
{
}

void PluginSelection::readConfig()
{
  const KTrader::OfferList offers = KTrader::self()->query(
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
    mPluginCombo->insertItem( service->name() );
    mPluginList.append( service );

    if ( service->property("X-KDE-PluginInfo-Name").toString() == mItem->value() )
      activeComponent = mPluginList.count() - 1;
  }

  mPluginCombo->setCurrentItem( activeComponent );
}

void PluginSelection::writeConfig()
{
  KService::Ptr ptr = *( mPluginList.at( mPluginCombo->currentItem() ) );
  mItem->setValue( ptr->property("X-KDE-PluginInfo-Name").toString() );
}

void PluginSelection::itemClicked( QListViewItem *item )
{
  if ( item )
    emit changed();
}

QValueList<QWidget *> PluginSelection::widgets() const
{
  QValueList<QWidget *> widgets;
  widgets.append( mPluginCombo );

  return widgets;
}

#include "kcmkontact.moc"
