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

#include <qlayout.h>
#include <qlabel.h>
#include <qbuttongroup.h>

#include <kdepimmacros.h>

extern "C"
{
  KDE_EXPORT KCModule *create_kontactconfig( QWidget *parent, const char * ) {
    return new KcmKontact( parent, "kcmkontact" );
  }
}

class PluginItem : public QCheckListItem
{
  public:
    PluginItem( const KService::Ptr &ptr, QListView *parent,
                const QString &text )
      : QCheckListItem( parent, text, QCheckListItem::CheckBox ),
        mPtr( ptr )
    {
    }

    KService::Ptr servicePtr() const
    {
      return mPtr;
    }

    virtual QString text( int column ) const
    {
      if ( column == 0 )
        return mPtr->name();
      else if ( column == 1 )
        return mPtr->comment();
      else
        return QString::null;
    }

  private:
    KService::Ptr mPtr;
};

KcmKontact::KcmKontact( QWidget *parent, const char *name )
  : KPrefsModule( Kontact::Prefs::self(), parent, name )
{
#if 0
  QVBoxLayout *topLayout = new QVBoxLayout( this );

  KPrefsWidRadios *radios = addWidRadios( Kontact::Prefs::self()->sidePaneTypeItem(),
                                          this );
  topLayout->addWidget( radios->groupBox() );
#endif

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


PluginSelection::PluginSelection( const QString &text, QStringList &reference,
                                  QWidget *parent )
  : mReference( reference )
{
  mBox = new QGroupBox( 0, Qt::Vertical, text, parent );
  QVBoxLayout *boxLayout = new QVBoxLayout( mBox->layout() );
  boxLayout->setAlignment( Qt::AlignTop );

  mPluginView = new KListView( mBox );
  mPluginView->setAllColumnsShowFocus( true );
  mPluginView->addColumn( i18n( "Name" ) );
  mPluginView->addColumn( i18n( "Description" ) );
  boxLayout->addWidget( mPluginView );

  connect( mPluginView, SIGNAL( clicked( QListViewItem* ) ),
           SLOT( itemClicked( QListViewItem* ) ) );
}

PluginSelection::~PluginSelection()
{
}

QGroupBox *PluginSelection::groupBox()  const
{
  return mBox;
}

void PluginSelection::readConfig()
{
  mPluginView->clear();
}

void PluginSelection::writeConfig()
{
  mReference.clear();

  QPtrList<QListViewItem> list;
  QListViewItemIterator it( mPluginView );
  while ( it.current() ) {
    PluginItem *item = static_cast<PluginItem*>( it.current() );
    if ( item ) {
      if ( item->isOn() )
        mReference.append( item->servicePtr()->
                           property( "X-KDE-KontactIdentifier" ).toString() );
    }
    ++it;
  }
}

void PluginSelection::itemClicked( QListViewItem *item )
{
  if ( item != 0 )
    emit changed();
}

#include "kcmkontact.moc"
