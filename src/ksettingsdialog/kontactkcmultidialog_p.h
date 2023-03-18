/*
    This file is part of the KDE project
    SPDX-FileCopyrightText: 2007 Matthias Kretz <kretz@kde.org>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-only
*/

#pragma once

#include "kcmutils_version.h"
#include "kontactkcmultidialog.h"
#include <QList>
#include <QStringList>

#if KCMUTILS_VERSION < QT_VERSION_CHECK(5, 240, 0)
class KCModuleProxy;
#else
class KCModule;
#endif
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

#if KCMUTILS_VERSION < QT_VERSION_CHECK(5, 240, 0)
    KCModuleProxy *currentModule = nullptr;
#else
    KCModule *currentModule = nullptr;
#endif

    struct CreatedModule {
#if KCMUTILS_VERSION < QT_VERSION_CHECK(5, 240, 0)
        KCModuleProxy *kcm = nullptr;
#else
        KCModule *kcm = nullptr;
#endif
        KPageWidgetItem *item = nullptr;
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
#if KCMUTILS_VERSION < QT_VERSION_CHECK(5, 240, 0)
    bool resolveChanges(KCModuleProxy *currentProxy);
    static bool moduleSave(KCModuleProxy *module);
#else
    bool resolveChanges(KCModule *currentProxy);
    static bool moduleSave(KCModule *module);
#endif
};
