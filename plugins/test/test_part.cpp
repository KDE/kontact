#include "test_part.h"


TestPart::TestPart(QObject *parent, const char *name)
  : KParts::ReadOnlyPart(parent, name)
{
  m_edit = new QMultiLineEdit;
  setWidget(m_edit);
}


bool TestPart::openFile()
{
  m_edit->setText(m_file);
  return true;
}


#include "test_part.moc"
