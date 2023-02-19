/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2007 Matthias Kretz <kretz@kde.org>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#pragma once

#include "kontactkcmultidialog.h"
#include <QList>
#include <QStringList>

class KCModuleProxy;
class KPageWidgetItem;

class KontactKCMultiDialogPrivate
{
    Q_DECLARE_PUBLIC(KontactKCMultiDialog)
protected:
    explicit KontactKCMultiDialogPrivate(KontactKCMultiDialog *parent)
        : q_ptr(parent)
    {
    }

    virtual ~KontactKCMultiDialogPrivate() = default;

    KCModuleProxy *currentModule = nullptr;

    struct CreatedModule {
        KCModuleProxy *kcm;
        KPageWidgetItem *item;
        QStringList componentNames;
    };

    using ModuleList = QList<CreatedModule>;
    ModuleList modules;

    void _k_slotCurrentPageChanged(KPageWidgetItem *current, KPageWidgetItem *previous);
    virtual void _k_clientChanged();
    void _k_dialogClosed();

    KontactKCMultiDialog *const q_ptr;

private:
    void init();
    void apply();
    bool resolveChanges(KCModuleProxy *currentProxy);
    static bool moduleSave(KCModuleProxy *module);
};
