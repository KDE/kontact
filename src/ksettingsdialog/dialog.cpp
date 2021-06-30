/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2003 Matthias Kretz <kretz@kde.org>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "dialog.h"
#include "dialog_p.h"

#include <KConfig>
#include <KLocalizedString>
#include <KPluginMetaData>
#include <KServiceGroup>
#include <KServiceTypeTrader>
#include <KSharedConfig>

#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QDir>
#include <QLabel>
#include <QPushButton>
#include <QStack>
#include <QVBoxLayout>

#include "kontact_debug.h"

namespace KSettings
{
Dialog::Dialog(QWidget *parent)
    : KCMultiDialog(*new DialogPrivate(this), new KPageWidget, parent)
{
}

Dialog::~Dialog()
{
}

void Dialog::addPluginComponent(const KPluginMetaData &parentPluginMetaData, const QVector<KPluginMetaData> &pluginMetaData)
{
    Q_D(Dialog);
    d->componentsMetaData.append({parentPluginMetaData, pluginMetaData});
}

void Dialog::showEvent(QShowEvent *)
{
    Q_D(Dialog);
    if (d->firstshow) {
        setUpdatesEnabled(false);
        d->createDialogFromServices();
        d->firstshow = false;
        setUpdatesEnabled(true);
    }
}

DialogPrivate::DialogPrivate(Dialog *parent)
    : KCMultiDialogPrivate(parent)
{
}

KPageWidgetItem *DialogPrivate::createPageItem(KPageWidgetItem *parentItem, const QString &name, const QString &comment, const QString &iconName)
{
    Q_Q(Dialog);
    QWidget *page = new QWidget(q);

    QLabel *iconLabel = new QLabel(page);
    QLabel *commentLabel = new QLabel(comment, page);
    commentLabel->setTextFormat(Qt::RichText);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->addWidget(iconLabel);
    layout->addWidget(commentLabel);
    layout->addStretch();

    KPageWidgetItem *item = new KPageWidgetItem(page, name);
    item->setIcon(QIcon::fromTheme(iconName));
    iconLabel->setPixmap(item->icon().pixmap(128, 128));

    const KPageWidgetModel *model = qobject_cast<const KPageWidgetModel *>(q->pageWidget()->model());
    Q_ASSERT(model);

    if (parentItem) {
        q->addSubPage(parentItem, item);
    } else {
        q->addPage(item);
    }

    return (item);
}

void DialogPrivate::createDialogFromServices()
{
    Q_Q(Dialog);

    for (const auto &pair : std::as_const(componentsMetaData)) {
        const KPluginMetaData &parentComponentMetaData = pair.first;
        const QVector<KPluginMetaData> &kcmsMetaData = pair.second;
        KPageWidgetItem *parentItem =
            createPageItem(nullptr, parentComponentMetaData.name(), parentComponentMetaData.description(), parentComponentMetaData.iconName());
        for (const KPluginMetaData &metaData : kcmsMetaData) {
            q->addModule(metaData, parentItem);
        }
    }

    QObject::connect(q, QOverload<const QByteArray &>::of(&KCMultiDialog::configCommitted), q, [](const QByteArray &componentName) {
        KSharedConfig::Ptr config = KSharedConfig::openConfig(QString::fromLatin1(componentName) + QLatin1String("rc"));
        config->reparseConfiguration();
    });
}

} // namespace

#include "moc_dialog.cpp"
