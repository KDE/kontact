#ifndef __NAVIGATOR_H__
#define __NAVIGATOR_H__

#include <klistbox.h>
#include <qlist.h>

class QPixmap;
class QListBox;

class EntryItem : public QListBoxPixmap
{
  public:
      EntryItem(QListBox *, const QPixmap&, const QString&, QObject*, const QCString&);
      ~EntryItem();
      /**
       * sets the icon for the item.
       * @param icon the icon to set
       * @param group the icongroup
       */
      void setIcon( const QString& icon, KIcon::Group group = KIcon::Panel );

      /* Does the virtal connection
       */
      void connectToReceiver( QObject * );

      /**
       * returns the width of this item.
       */
      virtual int width( const QListBox * ) const;
      /**
       * returns the height of this item.
       */
      virtual int height( const QListBox * ) const;

      /**
       * returns the pixmap.
       */
      virtual const QPixmap * pixmap() const {
        return m_Pixmap;
      }

  protected:
      virtual void paint( QPainter *p);

  private:
      QPixmap* m_Pixmap;
      QObject* m_receiver;
      QCString m_slot;
      QListBox* m_parent;
};

class Navigator : public KListBox
{
  Q_OBJECT

public:

  Navigator(QWidget *parent=0, const char *name=0);

  void addEntry(QString text, QString icon, QObject *receiver, const char *slot);


  QSize sizeHint() const;
  
private slots:

  void slotExecuted(QListBoxItem *item);


private:
/*
  QList<Entry> m_entries;

  int m_index;
*/

};

#endif
