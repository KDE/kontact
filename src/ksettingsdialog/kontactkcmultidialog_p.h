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

class KCModule;
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

    KCModule *currentModule = nullptr;

    struct CreatedModule {
        KCModule *kcm = nullptr;
        KPageWidgetItem *item = nullptr;
        QString pluginId;
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
    [[nodiscard]] bool resolveChanges(KCModule *currentProxy);
    static bool moduleSave(KCModule *module);
};
