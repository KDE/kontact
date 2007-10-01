/*
    This file is part of KDE Kontact.

    Copyright (c) 2004 Tobias Koenig <tokoe@kde.org>

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

#include <kaboutdata.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kdialog.h>
#include <kiconloader.h>
#include <klocale.h>
#include <plugin.h>
#include <kplugininfo.h>
#include <kservicetypetrader.h>
#include <kcomponentdata.h>
#include <QLayout>
#include <QLabel>
#include <QPixmap>
//Added by qt3to4:
#include <QVBoxLayout>

#include "kcmkontactsummary.h"

#include <kdemacros.h>

extern "C"
{
  KDE_EXPORT KCModule *create_kontactsummary( QWidget *parent, const char * ) {
    KComponentData inst("kcmkontactsummary");
    return new KCMKontactSummary( inst,parent );
  }
}

class PluginItem : public Q3CheckListItem
{
  public:
    PluginItem( const KPluginInfo &info, K3ListView *parent )
      : Q3CheckListItem( parent, QString(), Q3CheckListItem::CheckBox ),
        mInfo( info )
    {
      QPixmap pm = KIconLoader::global()->loadIcon( mInfo.icon(), KIconLoader::Small );
      setPixmap( 0, pm );
    }

    KPluginInfo pluginInfo() const
    {
      return mInfo;
    }

    virtual QString text( int column ) const
    {
      if ( column == 0 )
        return mInfo.name();
      else if ( column == 1 )
        return mInfo.comment();
      else
        return QString();
    }

  private:
    KPluginInfo mInfo;
};

PluginView::PluginView( QWidget *parent )
  : K3ListView( parent )
{
  addColumn( i18n( "Name" ) );
  setAllColumnsShowFocus( true );
  setFullWidth( true );
}

PluginView::~PluginView()
{
}

KCMKontactSummary::KCMKontactSummary( const KComponentData &inst, QWidget *parent )
  : KCModule( inst, parent )
{
  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->setSpacing( KDialog::spacingHint() );
  layout->setMargin( 0 );

  QLabel *label = new QLabel( i18n( "Here you can select which summary plugins to have visible in your summary view." ), this );
  layout->addWidget( label );

  mPluginView = new PluginView( this );
  layout->addWidget( mPluginView );

  layout->setStretchFactor( mPluginView, 1 );

  connect( mPluginView, SIGNAL( clicked( Q3ListViewItem* ) ),
           this, SLOT( itemClicked( Q3ListViewItem* ) ) );
  load();

  KAboutData *about = new KAboutData( I18N_NOOP( "kontactsummary" ), 0,
                                      ki18n( "KDE Kontact Summary" ),
                                      0, KLocalizedString(), KAboutData::License_GPL,
                                      ki18n( "(c), 2004 Tobias Koenig" ) );

  about->addAuthor( ki18n("Tobias Koenig"), KLocalizedString(), "tokoe@kde.org" );
  setAboutData( about );
}

void KCMKontactSummary::load()
{
  KService::List offers = KServiceTypeTrader::self()->query(
      QString::fromLatin1( "Kontact/Plugin" ),
      QString( "[X-KDE-KontactPluginVersion] == %1" ).arg( KONTACT_PLUGIN_VERSION ) );

  QStringList activeSummaries;

  KConfig config( "kontact_summaryrc" );
  if ( !config.hasKey( "ActiveSummaries" ) ) {
    activeSummaries << "kontact_kaddressbookplugin";
    activeSummaries << "kontact_specialdatesplugin";
    activeSummaries << "kontact_korganizerplugin";
    activeSummaries << "kontact_todoplugin";
    activeSummaries << "kontact_knotesplugin";
    activeSummaries << "kontact_kmailplugin";
    activeSummaries << "kontact_weatherplugin";
    activeSummaries << "kontact_newstickerplugin";
  } else {
    activeSummaries = config.readEntry( "ActiveSummaries" , QStringList() );
  }

  mPluginView->clear();

  KPluginInfo::List pluginList = KPluginInfo::fromServices( offers, KConfigGroup( &config, "Plugins" ) );
  KPluginInfo::List::Iterator it;
  for ( it = pluginList.begin(); it != pluginList.end(); ++it ) {
    it->load();

    if ( !it->isPluginEnabled() )
      continue;

    QVariant var = it->property( "X-KDE-KontactPluginHasSummary" );
    if ( !var.isValid() )
      continue;

    if ( var.toBool() == true ) {
      PluginItem *item = new PluginItem( *it, mPluginView );

      if ( activeSummaries.contains( it->pluginName() )  )
        item->setOn( true );
    }
  }
}

void KCMKontactSummary::save()
{
  QStringList activeSummaries;

  Q3ListViewItemIterator it( mPluginView, Q3ListViewItemIterator::Checked );
  while ( it.current() ) {
    PluginItem *item = static_cast<PluginItem*>( it.current() );
    activeSummaries.append( item->pluginInfo().pluginName() );
    ++it;
  }

  KConfig config( "kontact_summaryrc" );
  config.writeEntry( "ActiveSummaries", activeSummaries );
}

void KCMKontactSummary::defaults()
{
  emit changed( true );
}

void KCMKontactSummary::itemClicked( Q3ListViewItem* )
{
  emit changed( true );
}

#include "kcmkontactsummary.moc"
