#ifndef KNOTES_PLUGIN_H
#define KNOTES_PLUGIN_H


#include "kpplugin.h"

class KNotesPart;

class KNotesPlugin : public Kaplan::Plugin
{
  Q_OBJECT

public:

  KNotesPlugin(Kaplan::Core *core, const char *name, const QStringList &);
  ~KNotesPlugin();

  
private slots:
  
  void slotKNotesMenu();
  void slotShowNotes();
  
private:

  KNotesPart *m_part;
  
};


#endif
