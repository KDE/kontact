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
  
  void slotShowNotes();
  void slotNewNote();

private:

  void loadPart();
  KNotesPart *m_part;
  
};


#endif
