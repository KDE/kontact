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
#include <klocale.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qbuttongroup.h>

extern "C"
{
  KCModule *create_kontactconfig( QWidget *parent, const char * ) {
    return new KcmKontact( parent, "kcmkontact" );
  }
}

KcmKontact::KcmKontact( QWidget *parent, const char *name )
  : KPrefsModule( Kontact::Prefs::self(), parent, name )
{
  QVBoxLayout *topLayout = new QVBoxLayout( this );

  KPrefsWidRadios *radios = addWidRadios( i18n( "Side Pane Type" ),
                                          Kontact::Prefs::self()->mSidePaneType,
                                          this );
  radios->addRadio( i18n( "Button View with Sidebar Extension" ) );
  radios->addRadio( i18n( "Icon View" ) );
  new QLabel( i18n( "<em>You have to restart Kontact for this setting to take "
                    "effect.</em>" ), radios->groupBox() );
  topLayout->addWidget( radios->groupBox() );

  load();
}

const KAboutData* KcmKontact::aboutData() const
{
  KAboutData *about = new KAboutData( I18N_NOOP( "kontactconfig" ),
                                      I18N_NOOP( "KDE Kontact" ),
                                      0, 0, KAboutData::License_GPL,
                                      I18N_NOOP( "(c), 2003 Cornelius Schumacher" ) );

  about->addAuthor( "Cornelius Schumacher", 0, "schumacher@kde.org" );

  return about;
}

#include "kcmkontact.moc"
