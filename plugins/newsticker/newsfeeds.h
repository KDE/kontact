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
		NewsSourceData(
		QString::fromLatin1("superhits.ch"),
		QString::fromLatin1("http://www.superhits.ch/cgi-bin/superhits.cgi?page=rdf"),
		QString::fromLatin1("http://www.superhits.ch/favicon.ico"),
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
		QString::fromLatin1("linuxartist.org"),
		QString::fromLatin1("http://www.linuxartist.org/backend.php"),
		QString::fromLatin1("http://www.linuxartist.org/favicon.ico"),
		NewsSourceData::Computers ),
		NewsSourceData(
		QString::fromLatin1("KDE Deutschland"),
		QString::fromLatin1("http://www.kde.de/news/news.rdf"),
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
		NewsSourceData(
		QString::fromLatin1("GNOME News"),
		QString::fromLatin1("http://www.gnomedesktop.org/backend.php"),
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
		QString::fromLatin1("Linuxde.org"),
		QString::fromLatin1("http://www.linuxde.org/backends/news.rdf"),
		QString::fromLatin1("http://www.linuxde.org/favicon.ico"),
		NewsSourceData::Computers ),
		NewsSourceData(
		QString::fromLatin1("LinuxSecurity.com"),
		QString::fromLatin1("http://www.linuxsecurity.com/linuxsecurity_hybrid.rdf"),
		QString::null,
		NewsSourceData::Computers ),
		NewsSourceData(
		QString::fromLatin1("Linux Game Tome"),
		QString::fromLatin1("http://happypenguin.org/html/news.rdf"),
		QString::null,
		NewsSourceData::Computers ),
		NewsSourceData(
		QString::fromLatin1("Telefragged"),
		QString::fromLatin1("http://www.telefragged.com/cgi-bin/rdf.pl"),
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
		QString::fromLatin1("BSD Today"),
		QString::fromLatin1("http://bsdtoday.com/backend/bt.rdf"),
		QString::null,
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
		QString::fromLatin1("desktopian.org"),
		QString::fromLatin1("http://www.desktopian.org/includes/headlines.xml"),
		QString::fromLatin1("http://www.desktopian.org/favicon.ico"),
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
		QString::fromLatin1("LinuxNewbie"),
		QString::fromLatin1("http://www.linuxnewbie.org/news.cdf"),
		QString::fromLatin1("http://www.linuxnewbie.org/favicon.ico"),
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
		QString::fromLatin1("Linux4Us (sowie RadioTux & Magazin42)"),
		QString::fromLatin1("http://www.linux4us.de/newsticker.fd"),
		QString::null,
		NewsSourceData::Computers ),
		NewsSourceData(
		QString::fromLatin1("kdenews.unixcode.org"),
		QString::fromLatin1("http://kdenews.unixcode.org/?node=news&action=rss"),
		QString::null,
		NewsSourceData::Computers ),
		NewsSourceData(
		QString::fromLatin1("FreshPorts - the place for ports"),
		QString::fromLatin1("http://www.freshports.org/news.php3"),
		QString::fromLatin1("http://www.freshports.org/favicon.ico"),
		NewsSourceData::Computers ),
		NewsSourceData(
		QString::fromLatin1("NetPhoenix"),
		QString::fromLatin1("http://www.netphoenix.at/rss/netphoenix.php"),
		QString::fromLatin1("http://www.netphoenix.at/favicon.ico"),
		NewsSourceData::Computers ),
		NewsSourceData(
		QString::fromLatin1("ShortNews - by www.netphoenix.at"),
		QString::fromLatin1("http://www.netphoenix.at/rss/shortnews.php"),
		QString::fromLatin1("http://www.netphoenix.at/favicon.ico"),
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
		NewsSourceData(
		QString::fromLatin1("MaximumBSD"),
		QString::fromLatin1("http://www.maximumbsd.com/backend/mb.rdf"),
		QString::fromLatin1("http://www.maximumbsd.com/favicon.ico"),
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
		NewsSourceData(
		QString::fromLatin1("HotWired"),
		QString::fromLatin1("http://www.hotwired.com/webmonkey/meta/headlines.rdf"),
		QString::fromLatin1("http://www.hotwired.com/favicon.ico"),
		NewsSourceData::Misc ),
		NewsSourceData(
		QString::fromLatin1("The Register"),
		QString::fromLatin1("http://www.theregister.co.uk/tonys/slashdot.rdf"),
		QString::fromLatin1("http://www.theregister.co.uk/favicon.ico"),
		NewsSourceData::Misc ),
		NewsSourceData(
		QString::fromLatin1( "Christian Science Monitor" ),
		QString::fromLatin1( "http://www.csmonitor.com/rss/csm.rss"),
		QString::fromLatin1( "http://www.csmonitor.com/favicon.ico"),
		NewsSourceData::Misc ),
	// Recreation
		NewsSourceData(
		QString::fromLatin1("Segfault"),
		QString::fromLatin1("http://segfault.org/stories.xml"),
		QString::fromLatin1("http://segfault.org/favicon.ico"),
		NewsSourceData::Recreation ),
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
