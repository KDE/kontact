/*
  This file is part of the KDE Kontact.

  SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2008 Rafael Fernández López <ereslibre@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "sidepanebase.h"

#include <QListView>

namespace KontactInterface
{
class Core;
class Plugin;
}

class QAction;

namespace Kontact
{
class Model;
class MainWindow;
class Navigator;

class Navigator : public QListView
{
    Q_OBJECT

public:
    explicit Navigator(SidePaneBase *parent = nullptr);

    void updatePlugins(const QList<KontactInterface::Plugin *> &plugins);
    void setCurrentPlugin(const QString &plugin);

    int iconSize() const
    {
        return mIconSize;
    }

    bool showIcons() const
    {
        return mShowIcons;
    }

    bool showText() const
    {
        return mShowText;
    }

    void setMainWindow(MainWindow *mainWindow)
    {
        mMainWindow = mainWindow;
    }

    MainWindow *mainWindow()
    {
        return mMainWindow;
    }

    QSize sizeHint() const override;

Q_SIGNALS:
    void pluginActivated(KontactInterface::Plugin *plugin);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void showEvent(QShowEvent *event) override;

private Q_SLOTS:
    void slotCurrentChanged(const QModelIndex &current);
    void slotActionTriggered(QAction *checked);
    void slotHideSideBarTriggered();
    void updateNavigatorSize();

private:
    void setHelpText(QAction *act, const QString &text);
    SidePaneBase *mSidePane = nullptr;
    MainWindow *mMainWindow = nullptr;
    Model *mModel = nullptr;

    int mIconSize = 0;
    bool mShowIcons = false;
    bool mShowText = false;

    QAction *mShowIconsAction = nullptr;
    QAction *mShowTextAction = nullptr;
    QAction *mShowBothAction = nullptr;
    QAction *mBigIconsAction = nullptr;
    QAction *mNormalIconsAction = nullptr;
    QAction *mSmallIconsAction = nullptr;
    QAction *mHideSideBarAction = nullptr;
};

class IconSidePane : public SidePaneBase
{
    Q_OBJECT

public:
    IconSidePane(KontactInterface::Core *core, QWidget *parent);
    ~IconSidePane() override;

    void setCurrentPlugin(const QString &plugin) override;

public Q_SLOTS:
    void updatePlugins() override;

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    Navigator *mNavigator = nullptr;
};
}

