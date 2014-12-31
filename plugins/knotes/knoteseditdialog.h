/*
  Copyright (c) 2013-2015 Montel Laurent <montel@kde.org>

  This program is free software; you can redistribute it and/or modify it
  under the terms of the GNU General Public License, version 2, as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef KNOTESEDITDIALOG_H
#define KNOTESEDITDIALOG_H

#include <QDialog>
#include <KXMLGUIClient>

class KNoteEdit;
class KToolBar;
class QLineEdit;
class QPushButton;
class KNoteEditDialog : public QDialog, virtual public KXMLGUIClient
{
    Q_OBJECT
public:
    explicit KNoteEditDialog(bool readOnly, QWidget *parent = Q_NULLPTR);
    ~KNoteEditDialog();

    void setAcceptRichText(bool b);
    QString text() const;

    void setText(const QString &text);

    QString title() const;

    void setTitle(const QString &text);

    KNoteEdit *noteEdit() const;

    void setReadOnly(bool b);

    void setTabSize(int size);

    void setAutoIndentMode(bool newmode);

    void setTextFont(const QFont &font);

    void setColor(const QColor &fg, const QColor &bg);

    void setCursorPositionFromStart(int pos);

    int cursorPositionFromStart() const;
private Q_SLOTS:
    void slotTextChanged(const QString &text);

private:
    void init(bool readOnly);
    void readConfig();
    void writeConfig();
    QLineEdit *mTitleEdit;
    KNoteEdit *mNoteEdit;
    KToolBar *mTool;
    QPushButton *mOkButton;
};

#endif // KNOTESEDITDIALOG_H
