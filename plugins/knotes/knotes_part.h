#ifndef KNOTE_PART_H
#define KNOTE_PART_H

#include <klistview.h>
#include <qmap.h>

#include <kparts/part.h>


typedef QMap<int, QString> QNotesMap;

class QListViewItem;

class KNotesPart : public KParts::ReadOnlyPart
{
  Q_OBJECT

public:

  KNotesPart(QObject *parent=0, const char *name=0);
  
  bool openFile();

public slots:

  QNotesMap slotGetNotes();
  void slotOpenNote( QListViewItem* item);
  void slotInitPart();
  
private:

  KListView *m_listView;
  
};


#endif // KNOTE_PART_H
