#ifndef __TEST_PART_H__
#define __TEST_PART_H__



#include <kparts/part.h>
class QTextEdit;
class KAddressBookIface_stub;

class TestPart : public KParts::ReadOnlyPart
{
  Q_OBJECT

public:

  TestPart(QObject *parent=0, const char *name=0);
    ~TestPart();

protected:

  virtual bool openFile();
  bool connectToAddressBook();

protected slots:

  void newContact();
  void unregisteredFromDCOP( const QCString& );

private:

  QTextEdit *m_edit;
  KAddressBookIface_stub *m_kab_stub;

};


#endif
