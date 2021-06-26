/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2003 Matthias Kretz <kretz@kde.org>

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

#include <QCheckBox>
#include <QCoreApplication>
#include <QDialogButtonBox>
#include <QDir>
#include <QLabel>
#include <QPushButton>
#include <QStack>
#include <QVBoxLayout>

namespace KSettings
{
Dialog::Dialog(QWidget *parent)
    : Dialog(QStringList{}, parent)
{
}

Dialog::Dialog(const QStringList &components, QWidget *parent)
    : KCMultiDialog(*new DialogPrivate(this), new KPageWidget, parent)
{
    Q_D(Dialog);
    d->components = components;
}

Dialog::~Dialog()
{
}

void Dialog::addPluginComponent(const KPluginMetaData &parentPluginMetaData, const QVector<KPluginMetaData> &pluginMetaData)
{
    Q_D(Dialog);
    d->componentsMetaData.insert(parentPluginMetaData, pluginMetaData);
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

    for (const QString &compName : qAsConst(d->components)) {
        KSharedConfig::Ptr config = KSharedConfig::openConfig(compName + QLatin1String("rc"));
        config->sync();
    }
}

DialogPrivate::DialogPrivate(Dialog *parent)
    : KCMultiDialogPrivate(parent)
    , firstshow(true)
    , pluginStateDirty(0)
{
}

KPageWidgetItem *DialogPrivate::createPageItem(KPageWidgetItem *parentItem, const QString &name, const QString &comment, const QString &iconName, int weight)
{
    Q_Q(Dialog);
    QWidget *page = new QWidget(q);

    QCheckBox *checkBox = new QCheckBox(i18n("Enable component"), page);
    QLabel *iconLabel = new QLabel(page);
    QLabel *commentLabel = new QLabel(comment, page);
    commentLabel->setTextFormat(Qt::RichText);
    QVBoxLayout *layout = new QVBoxLayout(page);
    layout->addWidget(checkBox);
    layout->addWidget(iconLabel);
    layout->addWidget(commentLabel);
    layout->addStretch();

    KPageWidgetItem *item = new KPageWidgetItem(page, name);
    item->setIcon(QIcon::fromTheme(iconName));
    iconLabel->setPixmap(item->icon().pixmap(128, 128));
    item->setProperty("_k_weight", weight);
    checkBoxForItem.insert(item, checkBox);

    const KPageWidgetModel *model = qobject_cast<const KPageWidgetModel *>(q->pageWidget()->model());
    Q_ASSERT(model);

    if (parentItem) {
        const QModelIndex parentIndex = model->index(parentItem);
        const int siblingCount = model->rowCount(parentIndex);
        int row = 0;
        for (; row < siblingCount; ++row) {
            KPageWidgetItem *siblingItem = model->item(model->index(row, 0, parentIndex));
            if (siblingItem->property("_k_weight").toInt() > weight) {
                // the item we found is heavier than the new module
                q->insertPage(siblingItem, item);
                break;
            }
        }
        if (row == siblingCount) {
            // the new module is either the first or the heaviest item
            q->addSubPage(parentItem, item);
        }
    } else {
        const int siblingCount = model->rowCount();
        int row = 0;
        for (; row < siblingCount; ++row) {
            KPageWidgetItem *siblingItem = model->item(model->index(row, 0));
            if (siblingItem->property("_k_weight").toInt() > weight) {
                // the item we found is heavier than the new module
                q->insertPage(siblingItem, item);
                break;
            }
        }
        if (row == siblingCount) {
            // the new module is either the first or the heaviest item
            q->addPage(item);
        }
    }

    return (item);
}

void DialogPrivate::createDialogFromServices()
{
    Q_Q(Dialog);

    for (auto it = componentsMetaData.cbegin(), end = componentsMetaData.cend(); it != end; it++) {
        const KPluginMetaData &parentComponentMetaData = it.key();
        const QVector<KPluginMetaData> &kcmsMetaData = it.value();
        KPageWidgetItem *parentItem = createPageItem(nullptr,
                                                     parentComponentMetaData.name(),
                                                     parentComponentMetaData.description(),
                                                     parentComponentMetaData.iconName(),
                                                     parentComponentMetaData.value(QStringLiteral("X-KDE-Weight")).toInt());
        // connectItemCheckBox(item, pinfo, pinfo.isPluginEnabled());
        for (const KPluginMetaData &metaData : kcmsMetaData) {
            q->addModule(metaData, parentItem, QStringList());
        }
    }

    QObject::connect(q, QOverload<>::of(&KCMultiDialog::configCommitted), q, [this]() {
        updateConfiguration();
    });

    QObject::connect(q, QOverload<const QByteArray &>::of(&KCMultiDialog::configCommitted), q, [](const QByteArray &componentName) {
        KSharedConfig::Ptr config = KSharedConfig::openConfig(QString::fromLatin1(componentName) + QLatin1String("rc"));
        config->reparseConfiguration();
    });
}

void DialogPrivate::updateConfiguration()
{
    Q_Q(Dialog);
    const QHash<KPageWidgetItem *, KPluginInfo>::Iterator endIt = pluginForItem.end();
    QHash<KPageWidgetItem *, KPluginInfo>::Iterator it = pluginForItem.begin();
    for (; it != endIt; ++it) {
        KPageWidgetItem *item = it.key();
        KPluginInfo pinfo = it.value();
        pinfo.setPluginEnabled(item->isChecked());
        pinfo.save();
    }

    if (pluginStateDirty > 0) {
        Q_EMIT q->pluginSelectionChanged();
        pluginStateDirty = 0;
    }
}

void DialogPrivate::_k_clientChanged()
{
    if (pluginStateDirty > 0) {
        Q_Q(Dialog);
        q->buttonBox()->button(QDialogButtonBox::Apply)->setEnabled(true);
    } else {
        KCMultiDialogPrivate::_k_clientChanged();
    }
}

void DialogPrivate::_k_updateEnabledState(bool enabled)
{
    Q_Q(Dialog);
    KPageWidgetItem *item = qobject_cast<KPageWidgetItem *>(q->sender());
    if (!item) {
        qWarning() << "invalid sender";
        return;
    }

    // iterate over all child KPageWidgetItem objects and check whether they need to be enabled/disabled
    const KPageWidgetModel *model = qobject_cast<const KPageWidgetModel *>(q->pageWidget()->model());
    Q_ASSERT(model);
    QModelIndex index = model->index(item);
    if (!index.isValid()) {
        qWarning() << "could not find item in model";
        return;
    }

    const KPluginInfo &pinfo = pluginForItem.value(item);
    if (!pinfo.isValid()) {
        qWarning() << "could not find KPluginInfo in item";
        return;
    }
    if (pinfo.isPluginEnabled() != enabled) {
        ++pluginStateDirty;
    } else {
        --pluginStateDirty;
    }
    if (pluginStateDirty < 2) {
        _k_clientChanged();
    }

    QModelIndex firstborn = model->index(0, 0, index);
    if (firstborn.isValid()) {
        // change all children
        index = firstborn;
        QStack<QModelIndex> stack;
        while (index.isValid()) {
            KPageWidgetItem *item = model->item(index);
            item->setEnabled(enabled);
            firstborn = model->index(0, 0, index);
            if (firstborn.isValid()) {
                stack.push(index);
                index = firstborn;
            } else {
                index = index.sibling(index.row() + 1, 0);
                while (!index.isValid() && !stack.isEmpty()) {
                    index = stack.pop();
                    index = index.sibling(index.row() + 1, 0);
                }
            }
        }
    }
}

} // namespace

#include "moc_dialog.cpp"
