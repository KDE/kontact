/*
  This file is part of KDE Kontact.

  SPDX-FileCopyrightText: 2003 Cornelius Schumacher <schumacher@kde.org>
  SPDX-FileCopyrightText: 2008 Rafael Fernández López <ereslibre@kde.org>

  SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "iconsidepane.h"
#include "mainwindow.h"
#include "prefs.h"
using namespace Kontact;

#include <KontactInterface/Core>
#include <KontactInterface/Plugin>

#include <KIconLoader>
#include <KLocalizedString>
#include <QAction>
#include <QApplication>
#include <QIcon>

#include <QCollator>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QLayout>
#include <QPainter>
#include <QSortFilterProxyModel>
#include <QStringListModel>
#include <QStyledItemDelegate>
#include <QTimer>

namespace Kontact
{
class SelectionModel : public QItemSelectionModel
{
    Q_OBJECT
public:
    SelectionModel(QAbstractItemModel *model, QObject *parent)
        : QItemSelectionModel(model, parent)
    {
    }

public Q_SLOTS:
    void clear() override
    {
        // Don't allow the current selection to be cleared. QListView doesn't call to this method
        // nowadays, but just to cover of future change of implementation, since QTreeView does call
        // to this one when clearing the selection.
    }

    void select(const QModelIndex &index, QItemSelectionModel::SelectionFlags command) override
    {
        // Don't allow the current selection to be cleared
        if (!index.isValid() && (command & QItemSelectionModel::Clear)) {
            return;
        }
        QItemSelectionModel::select(index, command);
    }

    void select(const QItemSelection &selection, QItemSelectionModel::SelectionFlags command) override
    {
        // Don't allow the current selection to be cleared
        if (selection.isEmpty() && (command & QItemSelectionModel::Clear)) {
            return;
        }
        QItemSelectionModel::select(selection, command);
    }
};

class Model : public QStringListModel
{
    Q_OBJECT
public:
    enum SpecialRoles { PluginName = Qt::UserRole };

    explicit Model(Navigator *parentNavigator = nullptr)
        : QStringListModel(parentNavigator)
        , mNavigator(parentNavigator)
    {
    }

    void setPluginList(const QList<KontactInterface::Plugin *> &list)
    {
        pluginList = list;
    }

    Qt::ItemFlags flags(const QModelIndex &index) const override
    {
        Qt::ItemFlags flags = QStringListModel::flags(index);

        flags &= ~Qt::ItemIsEditable;

        if (index.isValid()) {
            if (static_cast<KontactInterface::Plugin *>(index.internalPointer())->disabled()) {
                flags &= ~Qt::ItemIsEnabled;
                flags &= ~Qt::ItemIsSelectable;
                flags &= ~Qt::ItemIsDropEnabled;
            } else {
                flags |= Qt::ItemIsDropEnabled;
            }
        } else {
            flags &= ~Qt::ItemIsDropEnabled;
        }

        return flags;
    }

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override
    {
        Q_UNUSED(parent)
        if (row < 0 || row >= pluginList.count()) {
            return QModelIndex();
        }
        return createIndex(row, column, pluginList[row]);
    }

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override
    {
        if (!index.isValid() || !index.internalPointer()) {
            return QVariant();
        }

        if (role == Qt::DisplayRole) {
            if (!mNavigator->showText()) {
                return QVariant();
            }
            return static_cast<KontactInterface::Plugin *>(index.internalPointer())->title();
        } else if (role == Qt::DecorationRole) {
            if (!mNavigator->showIcons()) {
                return QVariant();
            }
            return QIcon::fromTheme(static_cast<KontactInterface::Plugin *>(index.internalPointer())->icon());
        } else if (role == Qt::TextAlignmentRole) {
            return Qt::AlignCenter;
        } else if (role == Qt::ToolTipRole) {
            if (!mNavigator->showText()) {
                return static_cast<KontactInterface::Plugin *>(index.internalPointer())->title();
            }
            return QVariant();
        } else if (role == PluginName) {
            return static_cast<KontactInterface::Plugin *>(index.internalPointer())->identifier();
        }
        return QStringListModel::data(index, role);
    }

private:
    QList<KontactInterface::Plugin *> pluginList;
    Navigator *mNavigator = nullptr;
};

class SortFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit SortFilterProxyModel(QObject *parent = nullptr)
        : QSortFilterProxyModel(parent)
    {
        setDynamicSortFilter(true);
        sort(0);
    }

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override
    {
        auto leftPlugin = static_cast<KontactInterface::Plugin *>(left.internalPointer());
        auto rightPlugin = static_cast<KontactInterface::Plugin *>(right.internalPointer());

        if (leftPlugin->weight() == rightPlugin->weight()) {
            // Optimize it
            QCollator col;
            return col.compare(leftPlugin->title(), rightPlugin->title()) < 0;
        }

        return leftPlugin->weight() < rightPlugin->weight();
    }
};

class Delegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit Delegate(Navigator *parentNavigator = nullptr)
        : QStyledItemDelegate(parentNavigator)
        , mNavigator(parentNavigator)
    {
    }

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        if (!index.isValid() || !index.internalPointer()) {
            return;
        }

        QStyleOptionViewItem opt(*static_cast<const QStyleOptionViewItem *>(&option));
        // optionCopy.state |= QStyle::State_Active;
        opt.decorationPosition = QStyleOptionViewItem::Top;
        const int height = 0;
        painter->save();

        mNavigator->style()->drawControl(QStyle::CE_ItemViewItem, &opt, painter);

        if (mNavigator->showIcons() && mNavigator->showText()) {
            opt.icon = index.data(Qt::DecorationRole).value<QIcon>();
            const int size = mNavigator->iconSize();
            const auto spacing = mNavigator->style()->pixelMetric(QStyle::PM_FocusFrameHMargin);
            const int textHeight = painter->fontMetrics().height();

            const int y = opt.rect.y() + (opt.rect.height() - size - spacing - textHeight) / 2;

            opt.icon.paint(painter, QRect(opt.rect.x(), y, opt.rect.width(), size), Qt::AlignCenter, QIcon::Normal, QIcon::On);
            painter->drawText(QRect(opt.rect.x(), y + size + spacing, opt.rect.width(), textHeight), index.data(Qt::DisplayRole).toString(), {Qt::AlignCenter});
        } else if (mNavigator->showIcons()) {
            opt.icon = index.data(Qt::DecorationRole).value<QIcon>();
            const int size = mNavigator->iconSize() + height;
            opt.decorationSize = QSize(size, size);
            opt.icon.paint(painter, opt.rect, Qt::AlignCenter, QIcon::Normal, QIcon::On);
        } else if (mNavigator->showText()) {
            painter->drawText(opt.rect, index.data(Qt::DisplayRole).toString(), {Qt::AlignCenter});
        }
        painter->restore();
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        if (!index.isValid() || !index.internalPointer()) {
            return QSize();
        }

        QStyleOptionViewItem optionCopy(*static_cast<const QStyleOptionViewItem *>(&option));
        optionCopy.decorationPosition = QStyleOptionViewItem::Top;

        optionCopy.decorationSize = mNavigator->showIcons() ? QSize(mNavigator->iconSize(), mNavigator->iconSize()) : QSize();
        optionCopy.textElideMode = Qt::ElideNone;
        return QStyledItemDelegate::sizeHint(optionCopy, index);
    }

private:
    Navigator *mNavigator = nullptr;
};
}

Navigator::Navigator(SidePaneBase *parent)
    : QListView(parent)
    , mSidePane(parent)
    , mMainWindow(nullptr)
{
    setViewport(new QWidget(this));

    setVerticalScrollMode(ScrollPerPixel);
    setHorizontalScrollMode(ScrollPerPixel);
    setFrameShape(QFrame::NoFrame);

    mIconSize = Prefs::self()->sidePaneIconSize();
    mShowIcons = Prefs::self()->sidePaneShowIcons();
    mShowText = Prefs::self()->sidePaneShowText();

    auto viewMode = new QActionGroup(this);
    connect(viewMode, &QActionGroup::triggered, this, &Navigator::slotActionTriggered);

    mShowIconsAction = new QAction(i18nc("@action:inmenu", "Show Icons Only"), this);
    mShowIconsAction->setCheckable(true);
    mShowIconsAction->setActionGroup(viewMode);
    mShowIconsAction->setChecked(!mShowText && mShowIcons);
    setHelpText(mShowIconsAction, i18nc("@info:status", "Show sidebar items with icons and without text"));
    mShowIconsAction->setWhatsThis(i18nc("@info:whatsthis", "Choose this option if you want the sidebar items to have icons without text."));

    mShowTextAction = new QAction(i18nc("@action:inmenu", "Show Text Only"), this);
    mShowTextAction->setCheckable(true);
    mShowTextAction->setActionGroup(viewMode);
    mShowTextAction->setChecked(mShowText && !mShowIcons);
    setHelpText(mShowTextAction, i18nc("@info:status", "Show sidebar items with text and without icons"));
    mShowTextAction->setWhatsThis(i18nc("@info:whatsthis", "Choose this option if you want the sidebar items to have text without icons."));

    mShowBothAction = new QAction(i18nc("@action:inmenu", "Show Icons && Text"), this);
    mShowBothAction->setCheckable(true);
    mShowBothAction->setActionGroup(viewMode);
    mShowBothAction->setChecked(mShowText && mShowIcons);
    setHelpText(mShowBothAction, i18nc("@info:status", "Show sidebar items with icons and text"));
    mShowBothAction->setWhatsThis(i18nc("@info:whatsthis", "Choose this option if you want the sidebar items to have icons and text."));

    auto iconSize = new QActionGroup(this);
    connect(iconSize, &QActionGroup::triggered, this, &Navigator::slotActionTriggered);

    mBigIconsAction = new QAction(i18nc("@action:inmenu", "Big Icons"), this);
    mBigIconsAction->setCheckable(true);
    mBigIconsAction->setActionGroup(iconSize);
    mBigIconsAction->setChecked(mIconSize == KIconLoader::SizeLarge);
    setHelpText(mBigIconsAction, i18nc("@info:status", "Show large size sidebar icons"));
    mBigIconsAction->setWhatsThis(i18nc("@info:whatsthis", "Choose this option if you want the sidebar icons to be extra big."));

    mNormalIconsAction = new QAction(i18nc("@action:inmenu", "Normal Icons"), this);
    mNormalIconsAction->setCheckable(true);
    mNormalIconsAction->setActionGroup(iconSize);
    mNormalIconsAction->setChecked(mIconSize == KIconLoader::SizeMedium);
    setHelpText(mNormalIconsAction, i18nc("@info:status", "Show normal size sidebar icons"));
    mNormalIconsAction->setWhatsThis(i18nc("@info:whatsthis", "Choose this option if you want the sidebar icons to be normal size."));

    mSmallIconsAction = new QAction(i18nc("@action:inmenu", "Small Icons"), this);
    mSmallIconsAction->setCheckable(true);
    mSmallIconsAction->setActionGroup(iconSize);
    mSmallIconsAction->setChecked(mIconSize == KIconLoader::SizeSmallMedium);
    setHelpText(mSmallIconsAction, i18nc("@info:status", "Show small size sidebar icons"));
    mSmallIconsAction->setWhatsThis(i18nc("@info:whatsthis", "Choose this option if you want the sidebar icons to be extra small."));

    mHideSideBarAction = new QAction(i18nc("@action:inmenu", "Hide Sidebar"), this);
    mHideSideBarAction->setCheckable(true);
    mHideSideBarAction->setChecked(false);
    setHelpText(mHideSideBarAction, i18nc("@info:status", "Hide the icon sidebar"));
    mHideSideBarAction->setWhatsThis(i18nc("@info:whatsthis", "Choose this option if you want to hide the icon sidebar. Press F9 to unhide."));
    connect(mHideSideBarAction, &QAction::triggered, this, &Navigator::slotHideSideBarTriggered);

    auto sep = new QAction(this);
    sep->setSeparator(true);
    auto sep2 = new QAction(this);
    sep2->setSeparator(true);

    QList<QAction *> actionList;
    actionList << mShowIconsAction << mShowTextAction << mShowBothAction << sep << mBigIconsAction << mNormalIconsAction << mSmallIconsAction << sep2
               << mHideSideBarAction;

    insertActions(nullptr, actionList);

    setContextMenuPolicy(Qt::ActionsContextMenu);
    setViewMode(ListMode);
    setItemDelegate(new Delegate(this));
    mModel = new Model(this);
    auto sortFilterProxyModel = new SortFilterProxyModel(this);
    sortFilterProxyModel->setSourceModel(mModel);
    setModel(sortFilterProxyModel);
    setSelectionModel(new SelectionModel(sortFilterProxyModel, this));

    setDragDropMode(DropOnly);
    viewport()->setAcceptDrops(true);
    setDropIndicatorShown(true);

    connect(selectionModel(), &QItemSelectionModel::currentChanged, this, &Navigator::slotCurrentChanged);
}

void Navigator::updatePlugins(const QList<KontactInterface::Plugin *> &plugins_)
{
    QString currentPlugin;
    if (currentIndex().isValid()) {
        currentPlugin = currentIndex().model()->data(currentIndex(), Model::PluginName).toString();
    }

    QList<KontactInterface::Plugin *> pluginsToShow;
    for (KontactInterface::Plugin *plugin : plugins_) {
        if (plugin->showInSideBar()) {
            pluginsToShow << plugin;
        }
    }

    mModel->setPluginList(pluginsToShow);

    mModel->removeRows(0, mModel->rowCount());
    mModel->insertRows(0, pluginsToShow.count());

    // Restore the previous selected index, if any
    if (!currentPlugin.isEmpty()) {
        setCurrentPlugin(currentPlugin);
    }
}

void Navigator::setCurrentPlugin(const QString &plugin)
{
    const int numberOfRows(model()->rowCount());
    for (int i = 0; i < numberOfRows; ++i) {
        const QModelIndex index = model()->index(i, 0);
        const QString pluginName = model()->data(index, Model::PluginName).toString();

        if (plugin == pluginName) {
            selectionModel()->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);
            break;
        }
    }
}

QSize Navigator::sizeHint() const
{
    //### TODO: We can cache this value, so this reply is faster. Since here we won't
    //          have too many elements, it is not that important. When caching this value
    //          make sure it is updated correctly when new rows have been added or
    //          removed. (ereslibre)

    int maxWidth = 0;
    const int numberOfRows(model()->rowCount());
    for (int i = 0; i < numberOfRows; ++i) {
        const QModelIndex index = model()->index(i, 0);
        maxWidth = qMax(maxWidth, sizeHintForIndex(index).width());
    }

    // Take vertical scrollbar into account
    maxWidth += qApp->style()->pixelMetric(QStyle::PM_ScrollBarExtent);

    int viewHeight = QListView::sizeHint().height();

    QSize size(maxWidth + rect().width() - contentsRect().width(), viewHeight);
    return size;
}

void Navigator::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->proposedAction() == Qt::IgnoreAction) {
        return;
    }
    event->acceptProposedAction();
}

void Navigator::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->proposedAction() == Qt::IgnoreAction) {
        return;
    }

    const QModelIndex dropIndex = indexAt(event->pos());

    if (!dropIndex.isValid() || !(dropIndex.model()->flags(dropIndex) & Qt::ItemIsEnabled)) {
        event->setAccepted(false);
        return;
    } else {
        const QModelIndex sourceIndex = static_cast<const QSortFilterProxyModel *>(model())->mapToSource(dropIndex);
        auto plugin = static_cast<KontactInterface::Plugin *>(sourceIndex.internalPointer());
        if (!plugin->canDecodeMimeData(event->mimeData())) {
            event->setAccepted(false);
            return;
        }
    }

    event->acceptProposedAction();
}

void Navigator::dropEvent(QDropEvent *event)
{
    if (event->proposedAction() == Qt::IgnoreAction) {
        return;
    }

    const QModelIndex dropIndex = indexAt(event->pos());

    if (!dropIndex.isValid()) {
        return;
    } else {
        const QModelIndex sourceIndex = static_cast<const QSortFilterProxyModel *>(model())->mapToSource(dropIndex);
        auto plugin = static_cast<KontactInterface::Plugin *>(sourceIndex.internalPointer());
        plugin->processDropEvent(event);
    }
}

void Navigator::setHelpText(QAction *act, const QString &text)
{
    act->setStatusTip(text);
    act->setToolTip(text);
    if (act->whatsThis().isEmpty()) {
        act->setWhatsThis(text);
    }
}

void Navigator::showEvent(QShowEvent *event)
{
    parentWidget()->setMaximumWidth(sizeHint().width());
    parentWidget()->setMinimumWidth(sizeHint().width());

    QListView::showEvent(event);
}

void Navigator::slotCurrentChanged(const QModelIndex &current)
{
    if (!current.isValid() || !current.internalPointer() || !(current.model()->flags(current) & Qt::ItemIsEnabled)) {
        return;
    }

    QModelIndex source = static_cast<const QSortFilterProxyModel *>(current.model())->mapToSource(current);

    Q_EMIT pluginActivated(static_cast<KontactInterface::Plugin *>(source.internalPointer()));
}

void Navigator::slotActionTriggered(QAction *object)
{
    bool checked = object->isChecked();
    if (object == mShowIconsAction) {
        mShowIcons = checked;
        mShowText = !checked;
    } else if (object == mShowTextAction) {
        mShowIcons = !checked;
        mShowText = checked;
    } else if (object == mShowBothAction) {
        mShowIcons = checked;
        mShowText = checked;
    } else if (object == mBigIconsAction) {
        mIconSize = KIconLoader::SizeLarge;
    } else if (object == mNormalIconsAction) {
        mIconSize = KIconLoader::SizeMedium;
    } else if (object == mSmallIconsAction) {
        mIconSize = KIconLoader::SizeSmallMedium;
    }

    Prefs::self()->setSidePaneIconSize(mIconSize);
    Prefs::self()->setSidePaneShowIcons(mShowIcons);
    Prefs::self()->setSidePaneShowText(mShowText);

    reset();

    QTimer::singleShot(0, this, &Navigator::updateNavigatorSize);
}

void Navigator::slotHideSideBarTriggered()
{
    if (mainWindow()) {
        mainWindow()->showHideSideBar(false);
        mHideSideBarAction->setChecked(false);
    }
}

void Navigator::updateNavigatorSize()
{
    parentWidget()->setMaximumWidth(sizeHint().width());
    parentWidget()->setMinimumWidth(sizeHint().width());
    update();
}

IconSidePane::IconSidePane(KontactInterface::Core *core, QWidget *parent)
    : SidePaneBase(core, parent)
{
    mNavigator = new Navigator(this);
    layout()->addWidget(mNavigator);
    mNavigator->setFocusPolicy(Qt::NoFocus);
    mNavigator->setMainWindow(qobject_cast<MainWindow *>(core));
    connect(mNavigator, &Navigator::pluginActivated, this, &IconSidePane::pluginSelected);
}

IconSidePane::~IconSidePane()
{
}

void IconSidePane::setCurrentPlugin(const QString &plugin)
{
    mNavigator->setCurrentPlugin(plugin);
}

void IconSidePane::updatePlugins()
{
    mNavigator->updatePlugins(core()->pluginList());
}

void IconSidePane::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    const int newWidth(mNavigator->sizeHint().width());
    setFixedWidth(newWidth);
    mNavigator->setFixedWidth(newWidth);
}

#include "iconsidepane.moc"
