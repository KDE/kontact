#ifndef __NAVIGATOR_H__
#define __NAVIGATOR_H__


#include <klistbox.h>
#include <qlist.h>


class Entry;


class Navigator : public KListBox
{
  Q_OBJECT

public:

  Navigator(QWidget *parent=0, const char *name=0);

  void addEntry(QString text, QString icon, QObject *receiver, const char *slot);


private slots:

  void slotExecuted(QListBoxItem *item);


private:

  QList<Entry> m_entries;

  int m_index;

};


#endif
