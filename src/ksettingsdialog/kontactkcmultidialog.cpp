/*
    SPDX-FileCopyrightText: 2000 Matthias Elter <elter@kde.org>
    SPDX-FileCopyrightText: 2003 Daniel Molkentin <molkentin@kde.org>
    SPDX-FileCopyrightText: 2003, 2006 Matthias Kretz <kretz@kde.org>
    SPDX-FileCopyrightText: 2004 Frans Englich <frans.englich@telia.com>
    SPDX-FileCopyrightText: 2006 Tobias Koenig <tokoe@kde.org>
    SPDX-FileCopyrightText: 2021 Alexander Lohnau <alexander.lohnau@gmx.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kontactkcmultidialog.h"
#include "kontactkcmultidialog_p.h"
#include <KCModuleProxy>

#include <QApplication>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QJsonArray>
#include <QLayout>
#include <QProcess>
#include <QPushButton>
#include <QScreen>
#include <QStringList>
#include <QStyle>
#include <QUrl>

#include <KGuiItem>
#include <KIconUtils>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPageWidgetModel>

bool KontactKCMultiDialogPrivate::resolveChanges(KCModuleProxy *currentProxy)
{
    Q_Q(KontactKCMultiDialog);
    if (!currentProxy
        || !currentProxy->isChanged()) {

        return true;
    }

    // Let the user decide
    const int queryUser = KMessageBox::warningYesNoCancel(q,
                                                          i18n("The settings of the current module have changed.\n"
                                                               "Do you want to apply the changes or discard them?"),
                                                          i18n("Apply Settings"),
                                                          KStandardGuiItem::apply(),
                                                          KStandardGuiItem::discard(),
                                                          KStandardGuiItem::cancel());

    switch (queryUser) {
    case KMessageBox::Yes:
        return moduleSave(currentProxy);

    case KMessageBox::No:
        currentProxy->load();
        return true;

    case KMessageBox::Cancel:
        return false;

    default:
        Q_ASSERT(false);
        return false;
    }
}

void KontactKCMultiDialogPrivate::_k_slotCurrentPageChanged(KPageWidgetItem *current, KPageWidgetItem *previous)
{
    Q_Q(KontactKCMultiDialog);

    KCModuleProxy *previousModule = nullptr;
    for (int i = 0, total = modules.count(); i < total; ++i) {
        if (modules[i].item == previous) {
            previousModule = modules[i].kcm;
        }
    }

    // Delete global margins and spacing, since we want the contents to
    // be able to touch the edges of the window
    q->layout()->setContentsMargins({});

    const KPageWidget *pageWidget = q->pageWidget();
    pageWidget->layout()->setSpacing(0);

    // Then, we set the margins for the title header and the buttonBox footer
    const QStyle *style = q->style();
    const QMargins layoutMargins = QMargins(style->pixelMetric(QStyle::PM_LayoutLeftMargin),
                                            style->pixelMetric(QStyle::PM_LayoutTopMargin),
                                            style->pixelMetric(QStyle::PM_LayoutRightMargin),
                                            style->pixelMetric(QStyle::PM_LayoutBottomMargin));

    if (pageWidget->pageHeader()) {
        pageWidget->pageHeader()->setContentsMargins(layoutMargins);
    }

    // Do not set buttonBox's top margin as that space will be covered by the content's bottom margin
    q->buttonBox()->setContentsMargins(layoutMargins.left(), 0, layoutMargins.right(), layoutMargins.bottom());

    q->blockSignals(true);
    q->setCurrentPage(previous);

    if (resolveChanges(previousModule)) {
        q->setCurrentPage(current);
    }
    q->blockSignals(false);

    // We need to get the state of the now active module
    _k_clientChanged();
}

void KontactKCMultiDialogPrivate::_k_clientChanged()
{
    Q_Q(KontactKCMultiDialog);
    // qDebug();
    // Get the current module
    KCModuleProxy *activeModule = nullptr;
    for (int i = 0; i < modules.count(); ++i) {
        if (modules[i].item == q->currentPage()) {
            activeModule = modules[i].kcm;
            break;
        }
    }

    bool change = false;
    bool defaulted = false;
    if (activeModule) {
        change = activeModule->isChanged();
        defaulted = activeModule->defaulted();

        QPushButton *applyButton = q->buttonBox()->button(QDialogButtonBox::Apply);
        if (applyButton) {
            q->disconnect(applyButton, &QAbstractButton::clicked, q, &KontactKCMultiDialog::slotApplyClicked);
        }

        QPushButton *okButton = q->buttonBox()->button(QDialogButtonBox::Ok);
        if (okButton) {
            q->disconnect(okButton, &QAbstractButton::clicked, q, &KontactKCMultiDialog::slotOkClicked);
        }

        if (applyButton) {
            q->connect(applyButton, &QAbstractButton::clicked, q, &KontactKCMultiDialog::slotApplyClicked);
        }

        if (okButton) {
            q->connect(okButton, &QAbstractButton::clicked, q, &KontactKCMultiDialog::slotOkClicked);
        }
    }

    auto buttons = activeModule ? activeModule->buttons() : KCModule::NoAdditionalButton;

    QPushButton *resetButton = q->buttonBox()->button(QDialogButtonBox::Reset);
    if (resetButton) {
        resetButton->setVisible(buttons & KCModule::Apply);
        resetButton->setEnabled(change);
    }

    QPushButton *applyButton = q->buttonBox()->button(QDialogButtonBox::Apply);
    if (applyButton) {
        applyButton->setVisible(buttons & KCModule::Apply);
        applyButton->setEnabled(change);
    }

    QPushButton *cancelButton = q->buttonBox()->button(QDialogButtonBox::Cancel);
    if (cancelButton) {
        cancelButton->setVisible(buttons & KCModule::Apply);
    }

    QPushButton *okButton = q->buttonBox()->button(QDialogButtonBox::Ok);
    if (okButton) {
        okButton->setVisible(buttons & KCModule::Apply);
    }

    QPushButton *closeButton = q->buttonBox()->button(QDialogButtonBox::Close);
    if (closeButton) {
        closeButton->setHidden(buttons & KCModule::Apply);
    }

    QPushButton *helpButton = q->buttonBox()->button(QDialogButtonBox::Help);
    if (helpButton) {
        helpButton->setVisible(buttons & KCModule::Help);
    }

    QPushButton *defaultButton = q->buttonBox()->button(QDialogButtonBox::RestoreDefaults);
    if (defaultButton) {
        defaultButton->setVisible(buttons & KCModule::Default);
        defaultButton->setEnabled(!defaulted);
    }
}

void KontactKCMultiDialogPrivate::init()
{
    Q_Q(KontactKCMultiDialog);
    q->setFaceType(KPageDialog::Auto);
    q->setWindowTitle(i18nc("@title:window", "Configure"));
    q->setModal(false);

    auto buttonBox = new QDialogButtonBox(q);
    buttonBox->setStandardButtons(QDialogButtonBox::Help | QDialogButtonBox::RestoreDefaults | QDialogButtonBox::Cancel | QDialogButtonBox::Apply
                                  | QDialogButtonBox::Close | QDialogButtonBox::Ok | QDialogButtonBox::Reset);
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Ok), KStandardGuiItem::ok());
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Cancel), KStandardGuiItem::cancel());
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::RestoreDefaults), KStandardGuiItem::defaults());
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Apply), KStandardGuiItem::apply());
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Close), KStandardGuiItem::close());
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Reset), KStandardGuiItem::reset());
    KGuiItem::assign(buttonBox->button(QDialogButtonBox::Help), KStandardGuiItem::help());
    buttonBox->button(QDialogButtonBox::Close)->setVisible(false);
    buttonBox->button(QDialogButtonBox::Reset)->setEnabled(false);
    buttonBox->button(QDialogButtonBox::Apply)->setEnabled(false);

    q->connect(buttonBox->button(QDialogButtonBox::Apply), &QAbstractButton::clicked, q, &KontactKCMultiDialog::slotApplyClicked);
    q->connect(buttonBox->button(QDialogButtonBox::Ok), &QAbstractButton::clicked, q, &KontactKCMultiDialog::slotOkClicked);
    q->connect(buttonBox->button(QDialogButtonBox::RestoreDefaults), &QAbstractButton::clicked, q, &KontactKCMultiDialog::slotDefaultClicked);
    q->connect(buttonBox->button(QDialogButtonBox::Help), &QAbstractButton::clicked, q, &KontactKCMultiDialog::slotHelpClicked);
    q->connect(buttonBox->button(QDialogButtonBox::Reset), &QAbstractButton::clicked, q, &KontactKCMultiDialog::slotUser1Clicked);

    q->setButtonBox(buttonBox);
    q->connect(q, &KPageDialog::currentPageChanged, q, [this](KPageWidgetItem *current, KPageWidgetItem *before) {
        _k_slotCurrentPageChanged(current, before);
    });
}

KontactKCMultiDialog::KontactKCMultiDialog(QWidget *parent)
    : KPageDialog(parent)
    , d_ptr(new KontactKCMultiDialogPrivate(this))
{
    d_func()->init();
}

KontactKCMultiDialog::KontactKCMultiDialog(KontactKCMultiDialogPrivate &dd, KPageWidget *pageWidget, QWidget *parent, Qt::WindowFlags flags)
    : KPageDialog(pageWidget, parent, flags)
    , d_ptr(&dd)
{
    d_func()->init();
}

KontactKCMultiDialog::~KontactKCMultiDialog()
{
    delete d_ptr;
}

void KontactKCMultiDialog::showEvent(QShowEvent *ev)
{
    KPageDialog::showEvent(ev);
    adjustSize();
    /**
     * adjustSize() relies on sizeHint but is limited to 2/3 of the desktop size
     * Workaround for https://bugreports.qt.io/browse/QTBUG-3459
     *
     * We adjust the size after passing the show event
     * because otherwise window pos is set to (0,0)
     */
    QScreen *screen = QApplication::screenAt(pos());
    if (screen) {
        const QSize maxSize = screen->availableGeometry().size();
        resize(qMin(sizeHint().width(), maxSize.width()), qMin(sizeHint().height(), maxSize.height()));
    }
}

void KontactKCMultiDialog::slotDefaultClicked()
{
    Q_D(KontactKCMultiDialog);
    const KPageWidgetItem *item = currentPage();
    if (!item) {
        return;
    }

    for (int i = 0, total = d->modules.count(); i < total; ++i) {
        if (d->modules[i].item == item) {
            d->modules[i].kcm->defaults();
            d->_k_clientChanged();
            return;
        }
    }
}

void KontactKCMultiDialog::slotUser1Clicked()
{
    const KPageWidgetItem *item = currentPage();
    if (!item) {
        return;
    }

    Q_D(KontactKCMultiDialog);
    for (int i = 0; i < d->modules.count(); ++i) {
        if (d->modules[i].item == item) {
            d->modules[i].kcm->load();
            d->_k_clientChanged();
            return;
        }
    }
}

bool KontactKCMultiDialogPrivate::moduleSave(KCModuleProxy *module)
{
    if (!module) {
        return false;
    }

    module->save();
    return true;
}

void KontactKCMultiDialogPrivate::apply()
{
    Q_Q(KontactKCMultiDialog);
    QStringList updatedComponents;

    for (const CreatedModule &module : std::as_const(modules)) {
        KCModuleProxy *proxy = module.kcm;

        if (proxy->isChanged()) {
            proxy->save();
            /**
             * Add name of the components the kcm belongs to the list
             * of updated components.
             */
            const QStringList componentNames = module.componentNames;
            for (const QString &componentName : componentNames) {
                if (!updatedComponents.contains(componentName)) {
                    updatedComponents.append(componentName);
                }
            }
        }
    }

    // Send the configCommitted signal for every updated component.
    for (const QString &name : std::as_const(updatedComponents)) {
        Q_EMIT q->configCommitted(name.toLatin1());
    }
}

void KontactKCMultiDialog::slotApplyClicked()
{
    QPushButton *applyButton = buttonBox()->button(QDialogButtonBox::Apply);
    applyButton->setFocus();

    d_func()->apply();
}

void KontactKCMultiDialog::slotOkClicked()
{
    QPushButton *okButton = buttonBox()->button(QDialogButtonBox::Ok);
    okButton->setFocus();

    d_func()->apply();
    accept();
}

void KontactKCMultiDialog::slotHelpClicked()
{
    const KPageWidgetItem *item = currentPage();
    if (!item) {
        return;
    }

    Q_D(KontactKCMultiDialog);
    QString docPath;
    for (int i = 0; i < d->modules.count(); ++i) {
        if (d->modules[i].item == item) {
            docPath = d->modules[i].kcm->metaData().value(QStringLiteral("X-DocPath"));
            break;
        }
    }

    const QUrl docUrl = QUrl(QStringLiteral("help:/")).resolved(QUrl(docPath)); // same code as in KHelpClient::invokeHelp
    const QString docUrlScheme = docUrl.scheme();
    if (docUrlScheme == QLatin1String("help") || docUrlScheme == QLatin1String("man") || docUrlScheme == QLatin1String("info")) {
        QProcess::startDetached(QStringLiteral("khelpcenter"), QStringList() << docUrl.toString());
    } else {
        QDesktopServices::openUrl(docUrl);
    }
}

void KontactKCMultiDialog::closeEvent(QCloseEvent *event)
{
    Q_D(KontactKCMultiDialog);
    KPageDialog::closeEvent(event);

    /**
     * If we don't delete them, the DBUS registration stays, and trying to load the KCMs
     * in other situations will lead to "module already loaded in Foo," while to the user
     * doesn't appear so(the dialog is hidden)
     */
    for (auto &proxy : std::as_const(d->modules)) {
        proxy.kcm->deleteClient();
    }
}

KPageWidgetItem *KontactKCMultiDialog::addModule(const KPluginMetaData &metaData, KPageWidgetItem *parent)
{
    Q_D(KontactKCMultiDialog);
    // Create the scroller
    auto moduleScroll = new UnboundScrollArea(this);
    // Prepare the scroll area
    moduleScroll->setWidgetResizable(true);
    moduleScroll->setFrameStyle(QFrame::NoFrame);
    moduleScroll->viewport()->setAutoFillBackground(false);

    auto kcm = new KCModuleProxy(metaData, moduleScroll, QStringList());
    moduleScroll->setWidget(kcm);

    auto item = new KPageWidgetItem(moduleScroll, metaData.name());

    KontactKCMultiDialogPrivate::CreatedModule createdModule;
    createdModule.kcm = kcm;
    createdModule.item = item;
    d->modules.append(createdModule);

    // if (qobject_cast<KCModuleQml *>(kcm->realModule())) {
    // item->setHeaderVisible(false);
    //}

    if (kcm->realModule() && kcm->realModule()->useRootOnlyMessage()) {
        item->setHeader(QStringLiteral("<b>%1</b><br><i>%2</i>").arg(metaData.name(), kcm->realModule()->rootOnlyMessage()));
        item->setIcon(KIconUtils::addOverlay(QIcon::fromTheme(metaData.iconName()), QIcon::fromTheme(QStringLiteral("dialog-warning")), Qt::BottomRightCorner));
    } else {
        item->setHeader(metaData.name());
        item->setIcon(QIcon::fromTheme(metaData.iconName()));
    }
    bool updateCurrentPage = false;
    const auto model = qobject_cast<const KPageWidgetModel *>(pageWidget()->model());
    Q_ASSERT(model);
    if (parent) {
        addSubPage(parent, item);
    } else {
        addPage(item);
    }

    QObject::connect(kcm, qOverload<bool>(&KCModuleProxy::changed), this, [d]() {
        d->_k_clientChanged();
    });

    if (d->modules.count() == 1 || updateCurrentPage) {
        setCurrentPage(item);
        d->_k_clientChanged();
    }
    return item;
}

#include "moc_kontactkcmultidialog.cpp"
