
#include <dcopclient.h>
#include <kapplication.h>
#include <kdebug.h>
#include <klocale.h>
#include <krun.h>
#include <kstddirs.h>

#include "knotes_part.h"

class NotesListItem : public KListViewItem
{
public:
	NotesListItem( KListView * parent, int id, QString label1 ); 
    int id();	
private:
	int noteID;
};

NotesListItem::NotesListItem( KListView * parent, int id, QString label1 ):
	KListViewItem(parent, label1)
{
	noteID = id;
}

int NotesListItem::id()
{
	return noteID;
}


KNotesPart::KNotesPart(QObject *parent, const char *name)
  : KParts::ReadOnlyPart(parent, name)
{
  m_listView = new KListView;
  m_listView->addColumn(i18n("Title"));

  KRun *run = new KRun(locate("exe", "knotes")); // start kntoes if required
  connect(run, SIGNAL(finished()), SLOT(slotInitPart()));
  
  setWidget(m_listView);

   
}


bool KNotesPart::openFile()
{
	return false;
}

void KNotesPart::slotInitPart()
{
  QNotesMap map;
  map = slotGetNotes();
  QNotesMap::const_iterator it;
  for (it = map.begin(); it != map.end(); ++it )
  {
	 (void) new NotesListItem( m_listView, it.key(), it.data() );
  }
    
  connect(m_listView, SIGNAL(executed(QListViewItem*)), SLOT(slotOpenNote(QListViewItem*)) );
}

QNotesMap KNotesPart::slotGetNotes()
{
	QCString replyType;
	QByteArray data, replyData;
	QDataStream arg(  data, IO_WriteOnly );
	if( kapp->dcopClient()->call( "knotes", "KNotesIface", "notes()", data, replyType, replyData ) )
	{
		kdDebug() << "Reply Type: " << replyType << endl;
		QDataStream answer(  replyData, IO_ReadOnly );
		QNotesMap notes;
		answer >> notes;
		return notes;
	}
	else 
		return QNotesMap();
	
}

void KNotesPart::slotOpenNote( QListViewItem *item )
{
	int id = static_cast<NotesListItem*>( item )->id();

	QByteArray data;
	QDataStream arg( data, IO_WriteOnly );
	arg << id;
	if ( kapp->dcopClient()->send( "knotes", "KNotesIface", "showNote(int)", data ) )
		kdDebug() << "Opening Note!" << endl;		
		
}
#include "knotes_part.moc"
