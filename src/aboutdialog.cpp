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

#include "aboutdialog.h"
#include "aboutdialog.moc"

#include "core.h"
#include "plugin.h"

#include <klocale.h>
#include <kiconloader.h>
#include <kaboutdata.h>
#include <kactivelabel.h>
#include <ktextbrowser.h>

#include <qlayout.h>
#include <qlabel.h>

#include <kdebug.h>

using namespace Kontact;

AboutDialog::AboutDialog( Kontact::Core *core, const char *name )
  : KDialogBase( IconList, i18n("About Kontact"), Ok, Ok, core, name, false,
                 true ),
    mCore( core )
{
  addAboutData( i18n("Kontact Container Application"), QString( "kontact" ),
                KGlobal::instance()->aboutData() );

  QPtrList<Plugin> plugins = mCore->pluginList();
  uint i;
  for( i = 0; i < plugins.count(); ++i ) {
    addAboutPlugin( plugins.at( i ) );
  }

  addLicenseText( KGlobal::instance()->aboutData() );
}

void AboutDialog::addAboutPlugin( Kontact::Plugin *plugin )
{
  addAboutData( plugin->title(), plugin->icon(), plugin->aboutData() );
}

void AboutDialog::addAboutData( const QString &title, const QString &icon,
                                const KAboutData *about )
{
  QPixmap pixmap = KGlobal::iconLoader()->loadIcon( icon,
                                                    KIcon::Desktop, 48 );

  QFrame *topFrame = addPage( title, QString::null, pixmap );
  
  QBoxLayout *topLayout = new QVBoxLayout( topFrame );

  if ( !about ) {
    QLabel *label = new QLabel( i18n("No about information available."),
                                topFrame );
    topLayout->addWidget( label );
  } else {
    QString text;
    
    text += "<p><b>" + about->programName() + "</b><br>";
    
    text += i18n("Version %1</p>").arg( about->version() );


    if (!about->shortDescription().isEmpty()) {
      text += "<p>" + about->shortDescription() + "<br>" +
               about->copyrightStatement() + "</p>";
    }

    QString home = about->homepage();
    if ( !home.isEmpty() ) {
      text += "<a href=\"" + home + "\">" + home + "</a><br>";
    }

    QValueList<KAboutPerson> authors = about->authors();
    if ( !authors.isEmpty() ) {
      text += "<p><b>Authors:</b></p>";
    
      QValueList<KAboutPerson>::ConstIterator it;
      for( it = authors.begin(); it != authors.end(); ++it ) {
        text += (*it).name() + " &lt;<a href=\"mailto:" + (*it).emailAddress() + 
                "\">" + (*it).emailAddress() + "</a>&gt;<br>";
        if (!(*it).task().isEmpty()) text += "<i>" + (*it).task() + "</i><br>";
      }
    }
  
    QValueList<KAboutPerson> credits = about->credits();
    if ( !credits.isEmpty() ) {
      text += "<p><b>Thanks to:</b></p>";
    
      QValueList<KAboutPerson>::ConstIterator it;
      for( it = credits.begin(); it != credits.end(); ++it ) {
        text += (*it).name() + " &lt;<a href=\"mailto:" + (*it).emailAddress() + 
                "\">" + (*it).emailAddress() + "</a>&gt;<br>";
        if (!(*it).task().isEmpty()) text += "<i>" + (*it).task() + "</i><br>";
      }
    }
  
    QValueList<KAboutTranslator> translators = about->translators();
    if ( !translators.isEmpty() ) {
      text += "<p><b>Translators:</b></p>";
    
      QValueList<KAboutTranslator>::ConstIterator it;
      for( it = translators.begin(); it != translators.end(); ++it ) {
       text += (*it).name() + " &lt;<a href=\"mailto:" + (*it).emailAddress() + 
                "\">" + (*it).emailAddress() + "</a>&gt;<br>";
      }
    }
  
    KActiveLabel *label = new KActiveLabel( text, topFrame );
	label->setAlignment( AlignTop );
    topLayout->addWidget( label );
  }
}

void AboutDialog::addLicenseText(const KAboutData *about)
{
  if ( !about || about->license().isEmpty() ) return;
  
  QPixmap pixmap = KGlobal::iconLoader()->loadIcon( "scripting",
                                                    KIcon::Desktop, 48 );

  QString title = i18n("%1 license").arg(about->programName());
  
  QFrame *topFrame = addPage( title, QString::null, pixmap );
  QBoxLayout *topLayout = new QVBoxLayout( topFrame );
  
  KTextBrowser *textBrowser = new KTextBrowser( topFrame );
  textBrowser->setText( QString("<pre>%1</pre>").arg(about->license()) );
  
  topLayout->addWidget(textBrowser);
}
