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

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

  As a special exception, permission is given to link this program
  with any edition of Qt, and distribute the resulting executable,
  without including the source code for Qt in the source distribution.
*/

#include "aboutdialog.h"
#include "core.h"
#include "plugin.h"

#include <kdebug.h>
#include <klocale.h>
#include <kcomponentdata.h>
#include <kaboutdata.h>
#include <ktextbrowser.h>
#include <kicon.h>

#include <QLayout>
#include <QLabel>
#include <QTextEdit>

using namespace Kontact;

AboutDialog::AboutDialog( Kontact::Core *core )
  : KPageDialog( core ), mCore( core )
{
  setCaption( i18n( "About Kontact" ) );
  setButtons( Ok );
  setDefaultButton( Ok );
  setModal( false );
  showButtonSeparator( true );
  setFaceType( KPageDialog::List );
  addAboutData( i18n( "Kontact Container" ), QString( "kontact" ),
                KGlobal::mainComponent().aboutData() );

  QList<Plugin*> plugins = mCore->pluginList();
  QList<Plugin*>::ConstIterator end = plugins.constEnd();
  QList<Plugin*>::ConstIterator it = plugins.constBegin();
  for ( ; it != end; ++it ) {
    addAboutPlugin( *it );
  }

  addLicenseText( KGlobal::mainComponent().aboutData() );

  setInitialSize( QSize( 600, 400 ) );
  restoreDialogSize( KConfigGroup( KGlobal::config(), "AboutDialog" ) );
  connect( this, SIGNAL(finished(int)), this, SLOT(saveSize()) );
}

void AboutDialog::saveSize()
{
  KConfigGroup group( KGlobal::config(), "AboutDialog" );
  saveDialogSize( group );
  group.sync();
}

void AboutDialog::addAboutPlugin( Kontact::Plugin *plugin )
{
  addAboutData( plugin->title(), plugin->icon(), plugin->aboutData() );
}

void AboutDialog::addAboutData( const QString &title, const QString &icon,
                                const KAboutData *about )
{
  QIcon pixmap = KIcon( icon );

  QFrame *topFrame = new QFrame();
  KPageWidgetItem *pageItem = new KPageWidgetItem( topFrame, title );
  pageItem->setIcon( KIcon( pixmap ) );

  addPage( pageItem );

  QBoxLayout *topLayout = new QVBoxLayout( topFrame );

  if ( !about ) {
    QLabel *label = new QLabel( i18n( "No about information available." ), topFrame );
    topLayout->addWidget( label );
  } else {
    QString text;

    text += "<p>";
    text += "<b>" + about->programName() + "</b>";
    text += "<br>";

    text += i18n( "Version %1", about->version() );
    text += "</p>";

    if ( !about->shortDescription().isEmpty() ) {
      text += "<p>" + about->shortDescription() + "<br>" +
               about->copyrightStatement() + "</p>";
    }

    QString home = about->homepage();
    if ( !home.isEmpty() ) {
      text += "<a href=\"" + home + "\">" + home + "</a><br>";
    }

    text.replace( "\n", "<br>" );

    QLabel *label = new QLabel( text, topFrame );
    label->setAlignment( Qt::AlignTop );
    label->setOpenExternalLinks(true);
    label->setTextInteractionFlags(
      Qt::TextSelectableByMouse | Qt::TextSelectableByKeyboard | Qt::LinksAccessibleByMouse );
    topLayout->addWidget( label );

    QTextEdit *personView = new QTextEdit( topFrame ); //krazy:exclude=qclasses
    personView->setReadOnly( true );
    topLayout->addWidget( personView, 1 );

    text = "";

    text +="\
    <i>This Free Software product was improved as part of a commercial project:</i>\
    <h3>Credits</h3>\
    v20080902.2  Project Kowi (March 2007 - )<br /><br />\
    \
    <b>Production / Project Coordination</b><br />\
    Bernhard Reiter<br />\
    <b>Director of Development</b><br />\
    Till Adam\
    <br /><br />\
    \
    <table border=\"0\" width=\"100%\"> \
    <tr>\
     <td colspan=\"2\">\
      <b>Osnabr&uuml;ck Unit, Intevation GmbH</b>\
     </td>\
    </tr>\
    <tr>\
     <td width=\"60%\">\
      Unit Lead<br />\
      Senior QA, Packaging<br />\
      QA, Packaging<br />\
      Additional NSIS<br />\
      Backup Coordinator\
     </td>\
     <td>\
      Bernhard Reiter<br />\
      Bernhard Herzog<br />\
      Ludwig Reiter<br />\
      Emanuel Sch&uuml;tze<br />\
      Dr. Jan-Oliver Wagner\
     </td>\
    </tr>\
    </table>\
    <br /><br />\
    \
    <table border=\"0\" width=\"100%\">\
    <tr>\
     <td colspan=\"2\">\
        <b>Berlin Unit, Klar&auml;lvdalens Datakonsult AB</b>\
     </td>\
    </tr>\
    <tr>\
     <td width=\"60%\">\
      Unit Lead<br />\
      Development<br />\
      Development<br />\
      Additional D. + Crypto<br />\
      Crypto GUI Development\
     </td>\
     <td>\
      Till Adam<br />\
      Jaroslaw Staniek<br />\
      Volker Krause<br />\
      Frank Osterfeld<br />\
      Marc Mutz\
     </td>\
    </tr>\
    </table>\
    <br /><br />\
    \
    <table border=\"0\" width=\"100%\">\
    <tr>\
     <td colspan=\"2\">\
      <b>D&uuml;sseldorf Unit, g10 Code GmbH</b>\
     </td>\
    </tr>\
    <tr>\
     <td width=\"60%\">\
      Crypto-Backend Porting<br />\
      Crypto-Backend\
     </td>\
     <td>\
      Werner Koch<br />\
      Marcus Brinkmann\
     </td>\
    </tr>\
    </table>\
    <br /><br />\
    \
    <table border=\"0\" width=\"100%\">\
    <tr>\
     <td colspan=\"2\">\
      <b>External QA Darmstadt, basysKom GmbH</b>\
     </td>\
    </tr>\
    <tr>\
     <td width=\"60%\">\
      Unit-Lead\
     </td>\
     <td>\
      Dr. Stefan Werden\
     </td>\
    </tr>\
    </table><br /><br />";

    const QList<KAboutPerson> authors = about->authors();
    if ( !authors.isEmpty() ) {
      text += i18n( "<p><b>Authors:</b></p>" );

      QList<KAboutPerson>::ConstIterator it;
      for ( it = authors.begin(); it != authors.end(); ++it ) {
        text += formatPerson( (*it).name(), (*it).emailAddress() );
        if ( !(*it).task().isEmpty() ) {
          text += "<i>" + (*it).task() + "</i><br>";
        }
      }
    }

    const QList<KAboutPerson> credits = about->credits();
    if ( !credits.isEmpty() ) {
      text += i18n( "<p><b>Thanks to:</b></p>" );

      QList<KAboutPerson>::ConstIterator it;
      for ( it = credits.begin(); it != credits.end(); ++it ) {
        text += formatPerson( (*it).name(), (*it).emailAddress() );
        if ( !(*it).task().isEmpty() ) {
          text += "<i>" + (*it).task() + "</i><br>";
        }
      }
    }

    const QList<KAboutPerson> translators = about->translators();
    if ( !translators.isEmpty() ) {
      text += i18n( "<p><b>Translators:</b></p>" );

      QList<KAboutPerson>::ConstIterator it;
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
  if ( !about || about->license().isEmpty() ) {
    return;
  }

  QPixmap pixmap = KIconLoader::global()->loadIcon( "help-about",
                                                    KIconLoader::Desktop, 48 );

  QString title = i18n( "%1 License", about->programName() );

  QFrame *topFrame = new QFrame();
  KPageWidgetItem *page = new KPageWidgetItem( topFrame, title );
  page->setIcon( KIcon( pixmap ) );
  addPage( page );
  QBoxLayout *topLayout = new QVBoxLayout( topFrame );

  KTextBrowser *textBrowser = new KTextBrowser( topFrame );
  textBrowser->setHtml( QString( "<pre>%1</pre>" ).arg( about->license() ) );

  topLayout->addWidget( textBrowser );
}

#include "aboutdialog.moc"
