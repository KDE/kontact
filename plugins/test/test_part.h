#ifndef __TEST_PART_H__
#define __TEST_PART_H__


#include <qmultilineedit.h>


#include <kparts/part.h>


class TestPart : public KParts::ReadOnlyPart
{
  Q_OBJECT

public:

  TestPart(QObject *parent=0, const char *name=0);


protected:

  virtual bool openFile();
  

private:

  QMultiLineEdit *m_edit;
  
};


#endif
