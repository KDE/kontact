/*
    This file is part of Kontact.
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

#ifndef NEWSFEEDS_H
#define NEWSFEEDS_H

#include <qvaluelist.h>

#define DEFAULT_NEWSSOURCES 60

class NewsSourceData
{
  public:
    typedef QValueList<NewsSourceData> List;

    enum Category { Arts, Business, Computers, Misc,
                    Recreation, Society };

    NewsSourceData( const QString &name = I18N_NOOP( "Unknown" ),
                    const QString &url = QString::null,
                    const QString &icon = QString::null,
                    const Category category= Computers )
      : mName( name ), mURL( url ), mIcon( icon ), mCategory( category )
    {
    }

    QString name() const { return mName; }
    QString url() const { return mURL; }
    QString icon() const { return mIcon; }
    Category category() const { return mCategory; }

    QString mName;
    QString mURL;
    QString mIcon;
    Category mCategory;
};

static NewsSourceData NewsSourceDefault[DEFAULT_NEWSSOURCES] = {
  // Arts ---------------
    NewsSourceData(
    QString::fromLatin1("Bureau 42"),
    QString::fromLatin1("http://www.bureau42.com/rdf/"),
    QString::fromLatin1("http://www.bureau42.com/favicon.ico"),
    NewsSourceData::Arts ),
    NewsSourceData(
    QString::fromLatin1("eFilmCritic"),
    QString::fromLatin1("http://efilmcritic.com/fo.rdf"),
    QString::fromLatin1("http://efilmcritic.com/favicon.ico"),
    NewsSourceData::Arts ),
  // Business -----------
    NewsSourceData(
    QString::fromLatin1("Internet.com Business"),
    QString::fromLatin1("http://headlines.internet.com/internetnews/bus-news/news.rss"),
    QString::null,
    NewsSourceData::Business ),
    NewsSourceData(
    QString::fromLatin1("TradeSims"),
    QString::fromLatin1("http://www.tradesims.com/AEX.rdf"),
    QString::null,
    NewsSourceData::Business ),
  // Computers ----------
    NewsSourceData(
    QString::fromLatin1("KDE Deutschland"),
    QString::fromLatin1("http://www.kde.de/nachrichten/nachrichten.rdf"),
    QString::fromLatin1("http://www.kde.de/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    QString::fromLatin1("KDE France"),
    QString::fromLatin1("http://www.kde-france.org/backend-breves.php3"),
    QString::null,
    NewsSourceData::Computers ),
    NewsSourceData(
    QString::fromLatin1("FreeBSD Project News"),
    QString::fromLatin1("http://www.freebsd.org/news/news.rdf"),
    QString::fromLatin1("http://www.freebsd.org/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    QString::fromLatin1("dot.kde.org"),
    QString::fromLatin1("http://www.kde.org/dotkdeorg.rdf"),
    QString::fromLatin1("http://www.kde.org/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData( QString::fromLatin1("KDE-Look.org"),
                    QString::fromLatin1("http://www.kde.org/kde-look-content.rdf"),
                    QString::fromLatin1("http://kde-look.org/img/favicon-1-1.ico"),
                    NewsSourceData::Computers ),
    NewsSourceData( QString::fromLatin1("KDE-Apps.org"),
                    QString::fromLatin1("http://www.kde.org/dot/kde-apps-content.rdf"),
                    QString::fromLatin1("http://kde-apps.org/img/favicon-1-1.ico"),
                    NewsSourceData::Computers ),
    NewsSourceData( QString::fromLatin1("DesktopLinux"),
                    QString::fromLatin1("http://www.desktoplinux.com/backend/index.html"),
                    QString::fromLatin1("http://www.desktoplinux.com/images/favicon.ico"),
                    NewsSourceData::Computers ),
    NewsSourceData( QString::fromLatin1("DistroWatch"),
                    QString::fromLatin1("http://distrowatch.com/news/dw.xml"),
                    QString::fromLatin1("http://distrowatch.com/favicon.ico"),
                    NewsSourceData::Computers ),
    /*URL changed*/
    NewsSourceData(
    QString::fromLatin1("GNOME News"),
    QString::fromLatin1("http://www.gnomedesktop.org/node/feed"),
    QString::null,
    NewsSourceData::Computers ),
    NewsSourceData(
    QString::fromLatin1("Slashdot"),
    QString::fromLatin1("http://slashdot.org/slashdot.rdf"),
    QString::fromLatin1("http://slashdot.org/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    QString::fromLatin1("Ask Slashdot"),
    QString::fromLatin1("http://slashdot.org/askslashdot.rdf"),
    QString::fromLatin1("http://slashdot.org/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    QString::fromLatin1("Slashdot: Features"),
    QString::fromLatin1("http://slashdot.org/features.rdf"),
    QString::fromLatin1("http://slashdot.org/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    QString::fromLatin1("Slashdot: Apache"),
    QString::fromLatin1("http://slashdot.org/apache.rdf"),
    QString::fromLatin1("http://slashdot.org/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    QString::fromLatin1("Slashdot: Books"),
    QString::fromLatin1("http://slashdot.org/books.rdf"),
    QString::fromLatin1("http://slashdot.org/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    QString::fromLatin1("Jabber News"),
    QString::fromLatin1("http://www.jabber.org/news/rss.xml"),
    QString::null,
    NewsSourceData::Computers ),
    NewsSourceData(
    QString::fromLatin1("Freshmeat"),
    QString::fromLatin1("http://freshmeat.net/backend/fm.rdf"),
    QString::fromLatin1("http://freshmeat.net/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    QString::fromLatin1("Linux Weekly News"),
    QString::fromLatin1("http://www.lwn.net/headlines/rss"),
    QString::fromLatin1("http://www.lwn.net/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    QString::fromLatin1("heise online news"),
    QString::fromLatin1("http://www.heise.de/newsticker/heise.rdf"),
    QString::fromLatin1("http://www.heise.de/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    QString::fromLatin1("RUS-CERT Ticker"),
    QString::fromLatin1("http://cert.uni-stuttgart.de/ticker/rus-cert.rdf"),
    QString::fromLatin1("http://cert.uni-stuttgart.de/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    QString::fromLatin1("RUS-CERT Elsewhere"),
    QString::fromLatin1("http://cert.uni-stuttgart.de/ticker/rus-cert-elsewhere.rdf"),
    QString::fromLatin1("http://cert.uni-stuttgart.de/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    QString::fromLatin1("Kuro5hin"),
    QString::fromLatin1("http://kuro5hin.org/backend.rdf"),
    QString::fromLatin1("http://kuro5hin.org/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    QString::fromLatin1("Prolinux"),
    QString::fromLatin1("http://www.pl-forum.de/backend/pro-linux.rdf"),
    QString::fromLatin1("http://www.prolinux.de/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    QString::fromLatin1("LinuxSecurity.com"),
    QString::fromLatin1("http://www.linuxsecurity.com/linuxsecurity_hybrid.rdf"),
    QString::fromLatin1("http://www.linuxsecurity.com/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    QString::fromLatin1("Linux Game Tome"),
    QString::fromLatin1("http://happypenguin.org/html/news.rdf"),
    QString::null,
    NewsSourceData::Computers ),
    NewsSourceData(
    QString::fromLatin1("Mozilla"),
    QString::fromLatin1("http://www.mozilla.org/news.rdf"),
    QString::fromLatin1("http://www.mozillazine.org/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    QString::fromLatin1("MozillaZine"),
    QString::fromLatin1("http://www.mozillazine.org/contents.rdf"),
    QString::fromLatin1("http://www.mozillazine.org/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    QString::fromLatin1("Daemon News"),
    QString::fromLatin1("http://daily.daemonnews.org/ddn.rdf.php3"),
    QString::null,
    NewsSourceData::Computers ),
    NewsSourceData(
    QString::fromLatin1("use Perl;"),
    QString::fromLatin1("http://use.perl.org/useperl.rdf"),
    QString::null,
    NewsSourceData::Computers ),
    NewsSourceData(
    QString::fromLatin1("Root prompt"),
    QString::fromLatin1("http://www.rootprompt.org/rss/"),
    QString::fromLatin1("http://www.rootprompt.org/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    QString::fromLatin1("SecurityFocus"),
    QString::fromLatin1("http://www.securityfocus.com/topnews-rdf.html"),
    QString::fromLatin1("http://www.securityfocus.com/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    QString::fromLatin1("Arstechnica"),
    QString::fromLatin1("http://arstechnica.com/etc/rdf/ars.rdf"),
    QString::fromLatin1("http://arstechnica.com/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    QString::fromLatin1("amiga-news.de - deutschsprachige Amiga Nachrichten"),
    QString::fromLatin1("http://www.amiga-news.de/de/backends/news/index.rss"),
    QString::fromLatin1("http://www.amiga-news.de/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    QString::fromLatin1("amiga-news.de - english Amiga news"),
    QString::fromLatin1("http://www.amiga-news.de/en/backends/news/index.rss"),
    QString::fromLatin1("http://www.amiga-news.de/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    QString::fromLatin1("FreshPorts - the place for ports"),
    QString::fromLatin1("http://www.freshports.org/news.php3"),
    QString::fromLatin1("http://www.freshports.org/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    QString::fromLatin1("zez.org - about code "),
    QString::fromLatin1("http://zez.org/article/rssheadlines"),
    QString::null,
    NewsSourceData::Computers ),
    NewsSourceData(
    QString::fromLatin1("BSDatwork.com"),
    QString::fromLatin1("http://BSDatwork.com/backend.php"),
    QString::fromLatin1("http://BSDatwork.com/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    QString::fromLatin1("FreshSource - the place for source"),
    QString::fromLatin1("http://www.freshsource.org/news.php"),
    QString::fromLatin1("http://www.freshsource.org/favicon.ico"),
    NewsSourceData::Computers ),
    NewsSourceData(
    QString::fromLatin1("The FreeBSD Diary"),
    QString::fromLatin1("http://www.freebsddiary.org/news.php"),
    QString::fromLatin1("http://www.freebsddiary.org/favicon.ico"),
    NewsSourceData::Computers ),
  // Miscellaneous ------
    NewsSourceData(
    QString::fromLatin1("tagesschau.de"),
    QString::fromLatin1("http://www.tagesschau.de/newsticker.rdf"),
    QString::fromLatin1("http://www.tagesschau.de/favicon.ico"),
    NewsSourceData::Misc ),
    NewsSourceData(
    QString::fromLatin1("CNN Top Stories"),
    QString::fromLatin1("http://rss.cnn.com/rss/cnn_topstories.rss"),
    QString::fromLatin1("http://www.cnn.com/favicon.ico"),
    NewsSourceData::Misc ),
    /*feed URL changed*/
    NewsSourceData(
    QString::fromLatin1("HotWired"),
    QString::fromLatin1("http://www.wired.com/news/feeds/rss2/0,2610,,00.xml"),
    QString::fromLatin1("http://www.hotwired.com/favicon.ico"),
    NewsSourceData::Misc ),
    NewsSourceData(
    QString::fromLatin1("The Register"),
    QString::fromLatin1("http://www.theregister.co.uk/headlines.rss"),
    QString::fromLatin1("http://www.theregister.co.uk/favicon.ico"),
    NewsSourceData::Misc ),
    NewsSourceData(
    QString::fromLatin1( "Christian Science Monitor" ),
    QString::fromLatin1( "http://www.csmonitor.com/rss/csm.rss"),
    QString::fromLatin1( "http://www.csmonitor.com/favicon.ico"),
    NewsSourceData::Misc ),
  // Recreation
   // Society
    NewsSourceData(
    QString::fromLatin1("nippon.it"),
    QString::fromLatin1("http://www.nippon.it/backend.it.php"),
    QString::fromLatin1("http://www.nippon.it/favicon.ico"),
    NewsSourceData::Society ),
    NewsSourceData(
    QString::fromLatin1( "gflash" ),
    QString::fromLatin1( "http://www.gflash.de/backend.php"),
    QString::fromLatin1( "http://www.gflash.de/favicon.ico"),
    NewsSourceData::Society ),
    NewsSourceData(
    QString::fromLatin1( "Quintessenz" ),
    QString::fromLatin1( "http://quintessenz.at/cgi-bin/rdf"),
    QString::fromLatin1( "http://quintessenz.at/favicon.ico"),
    NewsSourceData::Society )
};

#endif
