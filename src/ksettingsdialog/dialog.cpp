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

uint qHash(const KCModuleInfo &info)
{
    return qHash(info.fileName());
}

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

void Dialog::setAllowComponentSelection(bool selection)
{
    d_func()->staticlistview = !selection;
}

bool Dialog::allowComponentSelection() const
{
    return !d_func()->staticlistview;
}

void Dialog::setKCMArguments(const QStringList &arguments)
{
    Q_D(Dialog);
    d->arguments = arguments;
}

void Dialog::setComponentBlacklist(const QStringList &blacklist)
{
    Q_D(Dialog);
    d->componentBlacklist = blacklist;
}

void Dialog::addPluginInfos(const KPluginInfo::List &plugininfos)
{
    Q_D(Dialog);
    for (KPluginInfo::List::ConstIterator it = plugininfos.begin(); it != plugininfos.end(); ++it) {
        d->registeredComponents.append(it->pluginName());
        const auto lst = it->kcmServices();
        if (lst.isEmpty()) {
            // this plugin has no kcm services, still we want to show the disable/enable stuff
            // so add a dummy kcm
            d->kcmInfos << KCModuleInfo(*it);
            continue;
        }
        for (const KService::Ptr &service : lst) {
            d->kcmInfos << KCModuleInfo(service);
        }
    }

    // The plugin, when disabled, disables all the KCMs described by kcmServices().
    // - Normally they are grouped using a .setdlg file so that the group parent can get a
    // checkbox to enable/disable the plugin.
    // - If the plugin does not belong to a group and has only one KCM the checkbox can be
    // used with this KCM.
    // - If the plugin belongs to a group but there are other modules in the group that do not
    // belong to this plugin we give a kError and show no checkbox
    // - If the plugin belongs to multiple groups we give a kError and show no checkbox
    d->plugininfos = plugininfos;
}

KPluginInfo::List Dialog::pluginInfos() const
{
    return d_func()->plugininfos;
}

void Dialog::showEvent(QShowEvent *)
{
    Q_D(Dialog);
    if (d->firstshow) {
        setUpdatesEnabled(false);
        d->kcmInfos += d->instanceServices();
        if (!d->components.isEmpty()) {
            d->kcmInfos += d->parentComponentsServices(d->components);
        }
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
    , staticlistview(true)
    , firstshow(true)
    , pluginStateDirty(0)
{
}

QSet<KCModuleInfo> DialogPrivate::instanceServices()
{
    // qDebug() ;
    QString componentName = QCoreApplication::instance()->applicationName();
    registeredComponents.append(componentName);
    // qDebug() << "calling KServiceGroup::childGroup( " << componentName << " )";
    KServiceGroup::Ptr service = KServiceGroup::childGroup(componentName);

    QSet<KCModuleInfo> ret;

    if (service && service->isValid()) {
        // qDebug() << "call was successful";
        const KServiceGroup::List list = service->entries();
        for (KServiceGroup::List::ConstIterator it = list.begin(); it != list.end(); ++it) {
            KSycocaEntry::Ptr p = (*it);
            if (p->isType(KST_KService)) {
                // qDebug() << "found service";
                const KService::Ptr service(static_cast<KService *>(p.data()));
                ret << KCModuleInfo(service);
            } else
                qWarning() << "KServiceGroup::childGroup returned"
                              " something else than a KService";
        }
    }

    return ret;
}

QSet<KCModuleInfo> DialogPrivate::parentComponentsServices(const QStringList &kcdparents)
{
    registeredComponents += kcdparents;
    QString constraint = kcdparents.join(QLatin1String("' in [X-KDE-ParentComponents]) or ('"));
    constraint = QStringLiteral("('") + constraint + QStringLiteral("' in [X-KDE-ParentComponents])");

    // qDebug() << "constraint = " << constraint;
    const QList<KService::Ptr> services = KServiceTypeTrader::self()->query(QStringLiteral("KCModule"), constraint);
    QSet<KCModuleInfo> ret;
    ret.reserve(services.count());
    for (const KService::Ptr &service : services) {
        ret << KCModuleInfo(service);
    }
    return ret;
}

bool DialogPrivate::isPluginForKCMEnabled(const KCModuleInfo *moduleinfo, KPluginInfo &pinfo) const
{
    // if the user of this class requested to hide disabled modules
    // we check whether it should be enabled or not
    bool enabled = true;
    // qDebug() << "check whether the '" << moduleinfo->moduleName() << "' KCM should be shown";
    // for all parent components
    const QStringList parentComponents = moduleinfo->property(QStringLiteral("X-KDE-ParentComponents")).toStringList();
    for (QStringList::ConstIterator pcit = parentComponents.begin(); pcit != parentComponents.end(); ++pcit) {
        // if the parentComponent is not registered ignore it
        if (!registeredComponents.contains(*pcit)) {
            continue;
        }

        // we check if the parent component is a plugin
        // if not the KCModule must be enabled
        enabled = true;
        if (pinfo.pluginName() == *pcit) {
            // it is a plugin: we check whether the plugin is enabled
            pinfo.load();
            enabled = pinfo.isPluginEnabled();
            // qDebug() << "parent " << *pcit << " is " << (enabled ? "enabled" : "disabled");
        }
        // if it is enabled we're done for this KCModuleInfo
        if (enabled) {
            return true;
        }
    }
    return enabled;
}

bool DialogPrivate::isPluginImmutable(const KPluginInfo &pinfo) const
{
    return pinfo.property(QStringLiteral("X-KDE-PluginInfo-Immutable")).toBool();
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

void DialogPrivate::parseGroupFile(const QString &filename)
{
    KConfig file(filename, KConfig::SimpleConfig);
    const QStringList groups = file.groupList();
    for (const QString &group : groups) {
        if (group.isEmpty()) {
            continue;
        }
        KConfigGroup conf(&file, group);

        const QString parentId = conf.readEntry("Parent");
        KPageWidgetItem *parentItem = pageItemForGroupId.value(parentId);
        KPageWidgetItem *item =
            createPageItem(parentItem, conf.readEntry("Name"), conf.readEntry("Comment"), conf.readEntry("Icon"), conf.readEntry("Weight", 100));
        pageItemForGroupId.insert(group, item);
    }
}

void DialogPrivate::createDialogFromServices()
{
    Q_Q(Dialog);
    // read .setdlg files   (eg: share/kapp/kapp.setdlg)
    const QString setdlgpath = QStandardPaths::locate(QStandardPaths::AppDataLocation /*includes appname, too*/,
                                                      QCoreApplication::instance()->applicationName() + QStringLiteral(".setdlg"));
    if (!setdlgpath.isEmpty()) {
        parseGroupFile(setdlgpath);
    }

    const QStringList dirs = QStandardPaths::locateAll(QStandardPaths::AppDataLocation, QStringLiteral("ksettingsdialog"), QStandardPaths::LocateDirectory);
    QMap<QString /*fileName*/, QString /*fullPath*/> fileMap;
    for (const QString &dir : dirs) {
        const QStringList fileNames = QDir(dir).entryList(QStringList() << QStringLiteral("*.setdlg"));
        for (const QString &file : fileNames) {
            if (!fileMap.contains(file)) {
                fileMap.insert(file, dir + QLatin1Char('/') + file);
            }
        }
    }
    for (auto it = fileMap.constBegin(); it != fileMap.constEnd(); ++it) {
        parseGroupFile(it.value());
    }

    // qDebug() << kcmInfos.count();
    for (const KCModuleInfo &info : qAsConst(kcmInfos)) {
        const QStringList parentComponents = info.property(QStringLiteral("X-KDE-ParentComponents")).toStringList();
        bool blacklisted = false;
        for (const QString &parentComponent : parentComponents) {
            if (componentBlacklist.contains(parentComponent)) {
                blacklisted = true;
                break;
            }
        }
        if (blacklisted) {
            continue;
        }
        const QString parentId = info.property(QStringLiteral("X-KDE-CfgDlgHierarchy")).toString();
        KPageWidgetItem *parent = pageItemForGroupId.value(parentId);
        if (!parent) {
            // dummy kcm
            bool foundPlugin = false;
            for (const KPluginInfo &pinfo : qAsConst(plugininfos)) {
                if (pinfo.libraryPath() == info.library()) {
                    if (pinfo.kcmServices().isEmpty()) {
                        // FIXME get weight from service or plugin info
                        const int weight = 1000;
                        KPageWidgetItem *item = createPageItem(nullptr, pinfo.name(), pinfo.comment(), pinfo.icon(), weight);
                        connectItemCheckBox(item, pinfo, pinfo.isPluginEnabled());
                        foundPlugin = true;
                        break;
                    }
                }
            }
            if (foundPlugin) {
                continue;
            }
        }
        KPageWidgetItem *item = q->addModule(info, parent, arguments);
        // qDebug() << "added KCM '" << info.moduleName() << "'";
        for (KPluginInfo pinfo : qAsConst(plugininfos)) {
            // qDebug() << pinfo.pluginName();
            if (pinfo.kcmServices().contains(info.service())) {
                const bool isEnabled = isPluginForKCMEnabled(&info, pinfo);
                // qDebug() << "correct KPluginInfo for this KCM";
                // this KCM belongs to a plugin
                if (parent && pinfo.kcmServices().count() >= 1) {
                    item->setEnabled(isEnabled);
                    const KPluginInfo &plugin = pluginForItem.value(parent);
                    if (plugin.isValid()) {
                        if (plugin != pinfo) {
                            qCritical() << "A group contains more than one plugin: '" << plugin.pluginName() << "' and '" << pinfo.pluginName()
                                        << "'. Now it won't be possible to enable/disable the plugin.";
                            parent->setCheckable(false);
                            q->disconnect(parent, SIGNAL(toggled(bool)), q, SLOT(_k_updateEnabledState(bool)));
                        }
                        // else everything is fine
                    } else {
                        connectItemCheckBox(parent, pinfo, isEnabled);
                    }
                } else {
                    pluginForItem.insert(item, pinfo);
                    item->setCheckable(!isPluginImmutable(pinfo));
                    item->setChecked(isEnabled);
                    q->connect(item, SIGNAL(toggled(bool)), q, SLOT(_k_updateEnabledState(bool)));
                }
                break;
            }
        }
    }
    // now that the KCMs are in, check for empty groups and remove them again
    {
        const KPageWidgetModel *model = qobject_cast<const KPageWidgetModel *>(q->pageWidget()->model());
        const QHash<QString, KPageWidgetItem *>::ConstIterator end = pageItemForGroupId.constEnd();
        QHash<QString, KPageWidgetItem *>::ConstIterator it = pageItemForGroupId.constBegin();
        for (; it != end; ++it) {
            const QModelIndex index = model->index(it.value());
            KPluginInfo pinfo;
            for (const KPluginInfo &p : qAsConst(plugininfos)) {
                if (p.name() == it.key()) {
                    pinfo = p;
                    break;
                }
            }
            bool allowEmpty = false;
            if (pinfo.isValid()) {
                allowEmpty = pinfo.property(QStringLiteral("X-KDE-PluginInfo-AllowEmptySettings")).toBool();
            }

            if (model->rowCount(index) == 0) {
                // no children, and it's not allowed => remove this item
                if (!allowEmpty) {
                    q->removePage(it.value());
                } else {
                    connectItemCheckBox(it.value(), pinfo, pinfo.isPluginEnabled());
                }
            }
        }
    }

    // TODO: Don't show the reset button until the issue with the
    // KPluginSelector::load() method is solved.
    // Problem:
    // KCMultiDialog::show() call KCModule::load() to reset all KCMs
    // (KPluginSelector::load() resets all plugin selections and all plugin
    // KCMs).
    // The reset button calls KCModule::load(), too but in this case we want the
    // KPluginSelector to only reset the current visible plugin KCM and not
    // touch the plugin selections.
    // I have no idea how to check that in KPluginSelector::load()...
    // q->showButton(KDialog::User1, true);

    QObject::connect(q, QOverload<>::of(&KCMultiDialog::configCommitted), q, [this]() {
        updateConfiguration();
    });

    QObject::connect(q, QOverload<const QByteArray &>::of(&KCMultiDialog::configCommitted), q, [](const QByteArray &componentName) {
        KSharedConfig::Ptr config = KSharedConfig::openConfig(QString::fromLatin1(componentName) + QLatin1String("rc"));
        config->reparseConfiguration();
    });
}

void DialogPrivate::connectItemCheckBox(KPageWidgetItem *item, const KPluginInfo &pinfo, bool isEnabled)
{
    Q_Q(Dialog);
    QCheckBox *checkBox = checkBoxForItem.value(item);
    Q_ASSERT(checkBox);
    pluginForItem.insert(item, pinfo);
    item->setCheckable(!isPluginImmutable(pinfo));
    item->setChecked(isEnabled);
    checkBox->setVisible(!isPluginImmutable(pinfo));
    checkBox->setChecked(isEnabled);
    q->connect(item, SIGNAL(toggled(bool)), q, SLOT(_k_updateEnabledState(bool)));
    q->connect(item, &KPageWidgetItem::toggled, checkBox, &QAbstractButton::setChecked);
    q->connect(checkBox, &QAbstractButton::clicked, item, &KPageWidgetItem::setChecked);
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

    // qDebug() ;

    QModelIndex firstborn = model->index(0, 0, index);
    if (firstborn.isValid()) {
        // qDebug() << "iterating over children";
        // change all children
        index = firstborn;
        QStack<QModelIndex> stack;
        while (index.isValid()) {
            // qDebug() << index;
            KPageWidgetItem *item = model->item(index);
            // qDebug() << "item->setEnabled(" << enabled << ')';
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
