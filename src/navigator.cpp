#include <qsignal.h>


#include <kglobal.h>
#include <kiconloader.h>


#include "navigator.h"


class Entry
{
public:

  QObject *receiver;
  QCString slot;
  int index;
  
};


Navigator::Navigator(QWidget *parent, const char *name)
  : KListBox(parent, name), m_index(0)
{
  m_entries.setAutoDelete(true);

  connect(this, SIGNAL(executed(QListBoxItem *)), this, SLOT(slotExecuted(QListBoxItem *)));
}


void Navigator::addEntry(QString text, QString icon, QObject *receiver, const char *slot)
{
  QPixmap pixmap = KGlobal::iconLoader()->loadIcon(icon, KIcon::Desktop, 48);
  insertItem(pixmap, text, m_index);

  Entry *e = new Entry;
  e->receiver = receiver;
  e->slot = slot;
  e->index = m_index;
  m_entries.append(e);

  m_index++;
}


void Navigator::slotExecuted(QListBoxItem *item)
{
  int i = index(item);
  if (i<0)
    return;

  QListIterator<Entry> it(m_entries);
  for ( ; it.current(); ++it)
    if (it.current()->index == i)
      {
        QSignal sig(this);
        sig.connect(it.current()->receiver, it.current()->slot);
        sig.activate();
        return;
      }
}


#include "navigator.moc"

