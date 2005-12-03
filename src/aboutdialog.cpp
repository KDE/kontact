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

#include "aboutdialog.h"

#include "core.h"
#include "plugin.h"

#include <klocale.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <kaboutdata.h>
#include <kactivelabel.h>
#include <ktextbrowser.h>

#include <qlayout.h>
#include <qlabel.h>
//Added by qt3to4:
#include <QPixmap>
#include <QVBoxLayout>
#include <Q3ValueList>
#include <Q3Frame>
#include <QBoxLayout>

#include <kdebug.h>

using namespace Kontact;

AboutDialog::AboutDialog( Kontact::Core *core, const char *name )
  : KDialogBase( IconList, i18n("About Kontact"), Ok, Ok, core, name, false,
                 true ),
    mCore( core )
{
  addAboutData( i18n( "Kontact Container" ), QString( "kontact" ),
                KGlobal::instance()->aboutData() );

  Q3ValueList<Plugin*> plugins = mCore->pluginList();
  Q3ValueList<Plugin*>::ConstIterator end = plugins.end();
  Q3ValueList<Plugin*>::ConstIterator it = plugins.begin();
  for ( ; it != end; ++it )
    addAboutPlugin( *it );

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
    QLabel *label = new QLabel( i18n( "No about information available." ),
                                topFrame );
    topLayout->addWidget( label );
  } else {
    QString text;

    text += "<p><b>" + about->programName() + "</b><br>";

    text += i18n( "Version %1</p>" ).arg( about->version() );

    if ( !about->shortDescription().isEmpty() ) {
      text += "<p>" + about->shortDescription() + "<br>" +
               about->copyrightStatement() + "</p>";
    }

    QString home = about->homepage();
    if ( !home.isEmpty() ) {
      text += "<a href=\"" + home + "\">" + home + "</a><br>";
    }

    text.replace( "\n", "<br>" );

    KActiveLabel *label = new KActiveLabel( text, topFrame );
    label->setAlignment( Qt::AlignTop );
    topLayout->addWidget( label );


    Q3TextEdit *personView = new Q3TextEdit( topFrame );
    personView->setReadOnly( true );
    topLayout->addWidget( personView, 1 );

    text = "";

    const Q3ValueList<KAboutPerson> authors = about->authors();
    if ( !authors.isEmpty() ) {
      text += i18n( "<p><b>Authors:</b></p>" );

      Q3ValueList<KAboutPerson>::ConstIterator it;
      for ( it = authors.begin(); it != authors.end(); ++it ) {
        text += formatPerson( (*it).name(), (*it).emailAddress() );
        if ( !(*it).task().isEmpty() )
          text += "<i>" + (*it).task() + "</i><br>";
      }
    }

    const Q3ValueList<KAboutPerson> credits = about->credits();
    if ( !credits.isEmpty() ) {
      text += i18n( "<p><b>Thanks to:</b></p>" );

      Q3ValueList<KAboutPerson>::ConstIterator it;
      for ( it = credits.begin(); it != credits.end(); ++it ) {
        text += formatPerson( (*it).name(), (*it).emailAddress() );
        if ( !(*it).task().isEmpty() )
          text += "<i>" + (*it).task() + "</i><br>";
      }
    }

    const Q3ValueList<KAboutTranslator> translators = about->translators();
    if ( !translators.isEmpty() ) {
      text += i18n("<p><b>Translators:</b></p>");

      Q3ValueList<KAboutTranslator>::ConstIterator it;
      for ( it = translators.begin(); it != translators.end(); ++it ) {
       text += formatPerson( (*it).name(), (*it).emailAddress() );
      }
    }

    personView->setText( text );
  }
}

QString AboutDialog::formatPerson( const QString &name, const QString &email )
{
  QString text = name;
  if ( !email.isEmpty() ) {
    text += " &lt;<a href=\"mailto:" + email + "\">" + email + "</a>&gt;";
  }

  text += "<br>";
  return text;
}

void AboutDialog::addLicenseText( const KAboutData *about )
{
  if ( !about || about->license().isEmpty() )
    return;

  QPixmap pixmap = KGlobal::iconLoader()->loadIcon( "signature",
                                                    KIcon::Desktop, 48 );

  QString title = i18n( "%1 License" ).arg( about->programName() );

  QFrame *topFrame = addPage( title, QString::null, pixmap );
  QBoxLayout *topLayout = new QVBoxLayout( topFrame );

  KTextBrowser *textBrowser = new KTextBrowser( topFrame );
  textBrowser->setText( QString( "<pre>%1</pre>" ).arg( about->license() ) );

  topLayout->addWidget( textBrowser );
}

#include "aboutdialog.moc"
