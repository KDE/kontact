/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2007 Matthias Kretz <kretz@kde.org>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#pragma once

#include "kontactkcmultidialog_p.h"
#include "kontactsettingsdialog.h"

#include <QString>

class KontactSettingsDialogPrivate : public KontactKCMultiDialogPrivate
{
    friend class PageNode;
    Q_DECLARE_PUBLIC(KontactSettingsDialog)
protected:
    KontactSettingsDialogPrivate(KontactSettingsDialog *parent);

    QStringList registeredComponents;
    QList<QPair<KPluginMetaData, QVector<KPluginMetaData>>> componentsMetaData;
    bool firstshow = true;

    KPageWidgetItem *createPageItem(KPageWidgetItem *parentItem, const QString &name, const QString &comment, const QString &iconName);

private:
    /**
     * @internal
     * This method is called only once. The KCMultiDialog is not created
     * until it's really needed. So if some method needs to access d->dlg it
     * checks for 0 and if it's not created this method will do it.
     */
    void createDialogFromServices();
};
