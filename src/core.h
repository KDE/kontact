#ifndef __CORE_H__
#define __CORE_H__


#include <qwidgetstack.h>


#include <kparts/mainwindow.h>
#include <kparts/part.h>
#include <kparts/partmanager.h>


#include "kpcore.h"


namespace Kaplan
{
  class Plugin;
};


class Navigator;
class KAction;

class Core : public Kaplan::Core
{
  Q_OBJECT

public:

  Core();
  ~Core();

  virtual void addMainEntry(QString text, QString icon, QObject *receiver, const char *slot);
  
  virtual void addPart(KParts::Part *part);

  virtual void showView(QWidget *view);

  virtual void insertNewAction(KAction *action);
  
private slots:

  void slotQuit();
  
  void activePartChanged(KParts::Part *part);

  void loadSettings();
  void saveSettings();
  
private:

  void loadPlugins();
  void addPlugin(Kaplan::Plugin *plugin);

  void setupActions();

  Navigator *m_navigator;

  KParts::PartManager *m_partManager;
  
  QWidgetStack *m_stack;

  KActionMenu *m_newActions;
  
};


#endif
