#ifndef __CORE_H__
#define __CORE_H__


#include <qwidgetstack.h>


#include <kparts/mainwindow.h>
#include <kparts/part.h>
#include <kparts/partmanager.h>
#include <klistbox.h>


#include "kpcore.h"


namespace Kaplan
{
  class Plugin;
};


class Navigator;


class Core : public Kaplan::Core
{
  Q_OBJECT

public:

  Core();
  ~Core();

  virtual void addMainEntry(QString text, QString icon, QObject *receiver, const char *slot);
  
  virtual void addPart(KParts::Part *part);

  virtual void showView(QWidget *view);


private slots:

  void slotQuit();
  
  void activePartChanged(KParts::Part *part);


private:

  void loadPlugins();
  void addPlugin(Kaplan::Plugin *plugin);

  void setupActions();

  Navigator *m_navigator;

  KParts::PartManager *m_partManager;
  
  QWidgetStack *m_stack;
  
};


#endif
