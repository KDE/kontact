/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2003 Matthias Kretz <kretz@kde.org>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#include "kontactsettingsdialog.h"
using namespace Qt::Literals::StringLiterals;

#include "kontactsettingsdialog_p.h"

#include <KLocalizedString>
#include <KPluginMetaData>
#include <KSharedConfig>

#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include "kontact_debug.h"

KontactSettingsDialog::KontactSettingsDialog(QWidget *parent)
    : KontactKCMultiDialog(*new KontactSettingsDialogPrivate(this), new KPageWidget, parent)
{
}

KontactSettingsDialog::~KontactSettingsDialog() = default;

void KontactSettingsDialog::addPluginComponent(const KPluginMetaData &parentPluginMetaData, const QList<KPluginMetaData> &pluginMetaData)
{
    Q_D(KontactSettingsDialog);
    d->componentsMetaData.append({parentPluginMetaData, pluginMetaData});
}

void KontactSettingsDialog::showEvent(QShowEvent *)
{
    Q_D(KontactSettingsDialog);
    if (d->firstshow) {
        setUpdatesEnabled(false);
        d->createDialogFromServices();
        d->firstshow = false;
        setUpdatesEnabled(true);
    }
}

KontactSettingsDialogPrivate::KontactSettingsDialogPrivate(KontactSettingsDialog *parent)
    : KontactKCMultiDialogPrivate(parent)
{
}

KPageWidgetItem *KontactSettingsDialogPrivate::createPageItem(KPageWidgetItem *parentItem, const QString &name, const QString &comment, const QString &iconName)
{
    Q_Q(KontactSettingsDialog);
    auto page = new QWidget(q);

    auto iconLabel = new QLabel(page);
    auto commentLabel = new QLabel(comment, page);
    commentLabel->setTextFormat(Qt::RichText);
    auto layout = new QVBoxLayout(page);
    layout->addWidget(iconLabel);
    layout->addWidget(commentLabel);
    layout->addStretch();

    auto item = new KPageWidgetItem(page, name);
    item->setIcon(QIcon::fromTheme(iconName));
    iconLabel->setPixmap(item->icon().pixmap(128, 128));

    const auto model = qobject_cast<const KPageWidgetModel *>(q->pageWidget()->model());
    Q_ASSERT(model);

    if (parentItem) {
        q->addSubPage(parentItem, item);
    } else {
        q->addPage(item);
    }

    return (item);
}

void KontactSettingsDialogPrivate::createDialogFromServices()
{
    Q_Q(KontactSettingsDialog);

    for (const auto &pair : std::as_const(componentsMetaData)) {
        const KPluginMetaData &parentComponentMetaData = pair.first;
        const QList<KPluginMetaData> &kcmsMetaData = pair.second;
        KPageWidgetItem *parentItem =
            createPageItem(nullptr, parentComponentMetaData.name(), parentComponentMetaData.description(), parentComponentMetaData.iconName());
        for (const KPluginMetaData &metaData : kcmsMetaData) {
            q->addModule(metaData, parentItem);
        }
    }

    QObject::connect(q, &KontactKCMultiDialog::configCommitted, q, [](const QByteArray &componentName) {
        KSharedConfig::Ptr config = KSharedConfig::openConfig(QString::fromLatin1(componentName) + "rc"_L1);
        config->reparseConfiguration();
    });
}

#include "moc_kontactsettingsdialog.cpp"
