/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2007 Matthias Kretz <kretz@kde.org>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KSETTINGS_DIALOG_P_H
#define KSETTINGS_DIALOG_P_H

#include "dialog.h"
#include "kcmoduleinfo.h"
#include "kcmultidialog_p.h"

#include <QHash>
#include <QSet>
#include <QString>

#include <KPageWidgetModel>
#include <KPluginInfo>
#include <KService>

class QCheckBox;

namespace KSettings
{
class DialogPrivate : public KCMultiDialogPrivate
{
    friend class PageNode;
    Q_DECLARE_PUBLIC(Dialog)
protected:
    DialogPrivate(Dialog *parent);

    QHash<QString, KPageWidgetItem *> pageItemForGroupId;
    QHash<KPageWidgetItem *, KPluginInfo> pluginForItem;
    QHash<KPageWidgetItem *, QCheckBox *> checkBoxForItem;
    KPluginInfo::List plugininfos;

    QStringList registeredComponents;
    QSet<KCModuleInfo> kcmInfos;
    QSet<KPluginMetaData> kcmsMetaData;
    QStringList componentBlacklist;
    QStringList arguments;
    QStringList components;

    bool staticlistview : 1;
    bool firstshow : 1;
    quint32 pluginStateDirty : 30;

    // void _k_configureTree();
    void _k_updateEnabledState(bool);
    void updateConfiguration();
    void _k_clientChanged() override;

    KPageWidgetItem *createPageItem(KPageWidgetItem *parentItem, const QString &name, const QString &comment, const QString &iconName, int weight);

    void connectItemCheckBox(KPageWidgetItem *item, const KPluginInfo &pinfo, bool isEnabled);

private:
    /**
     * @internal
     * Check whether the plugin associated with this KCM is enabled.
     */
    bool isPluginForKCMEnabled(const KCModuleInfo *moduleinfo, KPluginInfo &pinfo) const;
    bool isPluginImmutable(const KPluginInfo &pinfo) const;

    QSet<KCModuleInfo> instanceServices();
    QSet<KCModuleInfo> parentComponentsServices(const QStringList &);

    /**
     * @internal
     * Read the .setdlg file and add it to the groupmap
     */
    void parseGroupFile(const QString &);

    /**
     * @internal
     * If this module is put into a TreeList hierarchy this will return a
     * list of the names of the parent modules.
     */
    // QStringList parentModuleNames(KCModuleInfo *);

    /**
     * @internal
     * This method is called only once. The KCMultiDialog is not created
     * until it's really needed. So if some method needs to access d->dlg it
     * checks for 0 and if it's not created this method will do it.
     */
    void createDialogFromServices();
};

} // namespace KSettings
#endif // KSETTINGS_DIALOG_P_H
