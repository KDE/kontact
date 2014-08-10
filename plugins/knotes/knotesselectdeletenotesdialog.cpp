/*
  Copyright (c) 2013, 2014 Montel Laurent <montel@kde.org>

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

#include "knotesselectdeletenotesdialog.h"
#include "knotesiconview.h"

#include <KLocalizedString>
#include <KStandardGuiItem>


#include <QHBoxLayout>
#include <QLabel>
#include <KSharedConfig>
#include <QDialogButtonBox>
#include <KConfigGroup>
#include <QPushButton>
#include <QVBoxLayout>

KNotesSelectDeleteNotesListWidget::KNotesSelectDeleteNotesListWidget(QWidget *parent)
    : QListWidget(parent)
{

}

KNotesSelectDeleteNotesListWidget::~KNotesSelectDeleteNotesListWidget()
{

}

void KNotesSelectDeleteNotesListWidget::setItems(const QList<KNotesIconViewItem*> &items)
{
    Q_FOREACH (KNotesIconViewItem *item, items) {
        QListWidgetItem *i = new QListWidgetItem(this);
        if (item->readOnly()) {
            i->setText(item->realName() + QLatin1Char(' ') + i18n("(note locked, it will not removed)"));
            i->setTextColor(Qt::red);
        } else {
            i->setText(item->realName());
        }
    }
}

KNotesSelectDeleteNotesDialog::KNotesSelectDeleteNotesDialog(const QList<KNotesIconViewItem *> &items, QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle( i18nc( "@title:window", "Confirm Delete" ) );
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel);
    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &KNotesSelectDeleteNotesDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &KNotesSelectDeleteNotesDialog::reject);
    buttonBox->button(QDialogButtonBox::Cancel)->setDefault(true);
    setModal( true );
    QWidget *w = new QWidget;
    QVBoxLayout *lay = new QVBoxLayout;
    w->setLayout(lay);
    QLabel *lab = new QLabel( i18ncp( "@info", "Do you really want to delete this note?", "Do you really want to delete these %1 notes?", items.count() ));
    lay->addWidget(lab);
    mSelectedListWidget = new KNotesSelectDeleteNotesListWidget;
    lay->addWidget(mSelectedListWidget);
    mainLayout->addWidget(w);
    mSelectedListWidget->setItems(items);
    okButton->setText(KStandardGuiItem::del().text());
    
    mainLayout->addWidget(buttonBox);
    readConfig();
}

KNotesSelectDeleteNotesDialog::~KNotesSelectDeleteNotesDialog()
{
    writeConfig();
}

void KNotesSelectDeleteNotesDialog::readConfig()
{
    KConfigGroup grp( KSharedConfig::openConfig(), "KNotesSelectDeleteNotesDialog" );
    const QSize size = grp.readEntry( "Size", QSize(300, 200) );
    if ( size.isValid() ) {
        resize( size );
    }
}

void KNotesSelectDeleteNotesDialog::writeConfig()
{
    KConfigGroup grp( KSharedConfig::openConfig(), "KNotesSelectDeleteNotesDialog" );
    grp.writeEntry( "Size", size() );
    grp.sync();
}


