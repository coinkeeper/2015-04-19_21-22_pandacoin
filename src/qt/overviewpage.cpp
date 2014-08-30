#include "overviewpage.h"
#include "ui_overviewpage.h"

#include "walletmodel.h"
#include "bitcoinunits.h"
#include "optionsmodel.h"
#include "transactiontablemodel.h"
#include "guiutil.h"
#include "guiconstants.h"
#include "accountmodel.h"
#include "richtextdelegate.h"

#include <QAbstractItemDelegate>
#include <QPainter>
#include <QLocale>
#include <QDateTime>
#include <QSortFilterProxyModel>
#include <QListView>

#define DECORATION_SIZE 64
#define NUM_ITEMS 3
class TxViewDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
public:
    TxViewDelegate(): QAbstractItemDelegate(), unit(BitcoinUnits::BTC)
    {

    }

    inline void paint(QPainter *painter, const QStyleOptionViewItem &option,
                      const QModelIndex &index ) const
    {
        painter->save();

        QIcon icon = qvariant_cast<QIcon>(index.data(Qt::DecorationRole));
        QRect mainRect = option.rect;
        QRect decorationRect(mainRect.topLeft(), QSize(DECORATION_SIZE, DECORATION_SIZE));
        int xspace = DECORATION_SIZE + 8;
        int ypad = 6;
        int halfheight = (mainRect.height() - 2*ypad)/2;
        QRect amountRect(mainRect.left() + xspace, mainRect.top()+ypad, mainRect.width() - xspace, halfheight);
        QRect addressRect(mainRect.left() + xspace, mainRect.top()+ypad+halfheight, mainRect.width() - xspace, halfheight);
        icon.paint(painter, decorationRect);

        QDateTime date = index.data(TransactionTableModel::DateRole).toDateTime();
        QString address = index.data(Qt::DisplayRole).toString();
        qint64 amount = index.data(TransactionTableModel::AmountRole).toLongLong();
        bool confirmed = index.data(TransactionTableModel::ConfirmedRole).toBool();
        QVariant value = index.data(Qt::ForegroundRole);
        QColor foreground = option.palette.color(QPalette::Text);
        if(qVariantCanConvert<QColor>(value))
        {
            foreground = qvariant_cast<QColor>(value);
        }

        painter->setPen(foreground);
        painter->drawText(addressRect, Qt::AlignLeft|Qt::AlignVCenter, address);

        if(amount < 0)
        {
            foreground = COLOR_NEGATIVE;
        }
        else if(!confirmed)
        {
            foreground = COLOR_UNCONFIRMED;
        }
        else
        {
            foreground = option.palette.color(QPalette::Text);
        }
        painter->setPen(foreground);
        QString amountText = BitcoinUnits::formatWithUnit(unit, amount, true, true);
        if(!confirmed)
        {
            amountText = QString("[") + amountText + QString("]");
        }
        painter->drawText(amountRect, Qt::AlignRight|Qt::AlignVCenter, amountText);

        painter->setPen(option.palette.color(QPalette::Text));
        painter->drawText(amountRect, Qt::AlignLeft|Qt::AlignVCenter, GUIUtil::dateTimeStr(date));

        painter->restore();
    }

    inline QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        return QSize(DECORATION_SIZE, DECORATION_SIZE);
    }

    int unit;

};



OverviewPage::OverviewPage(QWidget *parent)
: QWidget(parent)
, ui(new Ui::OverviewPage)
, currentBalance(-1)
, currentStake(0)
, currentUnconfirmedBalance(-1)
//, txdelegate(new TxViewDelegate())
{
    ui->setupUi(this);

    connect(ui->portfolio_heading_more, SIGNAL(pressed()), this, SIGNAL(requestGotoTransactionPage()));
    connect(ui->PortfolioTable, SIGNAL(clicked(QModelIndex)), this, SLOT(handleAccountClicked(QModelIndex)));

    //fixme: hardcoded
    ui->welcome_heading->setText("Welcome to your PandaBank, You last logged on at " + QDateTime::currentDateTime().time().toString() + " on "+ QDateTime::currentDateTime().date().toString());

    //Hide for now (apparently this will only be visible in later versions of UI).
    ui->portfolio_overview_description->setVisible(false);
}

void OverviewPage::handleAccountClicked(const QModelIndex &index)
{
    if(model)
    {
        if(index.column() == AccountModel::Address || index.column() == AccountModel::Label)
        {
            emit accountClicked(ui->PortfolioTable->model()->data(index).toString());
        }
    }
}

OverviewPage::~OverviewPage()
{
    delete ui;
}

void OverviewPage::setBalance(qint64 balance, qint64 stake, qint64 unconfirmedBalance, qint64 immatureBalance)
{
    int unit = model->getOptionsModel()->getDisplayUnit();
    currentBalance = balance;
    currentStake = stake;
    currentUnconfirmedBalance = unconfirmedBalance;
    currentImmatureBalance = immatureBalance;
    ui->labelBalance->setText(formatBitcoinAmountAsRichString(BitcoinUnits::formatWithUnit(unit, balance, true, false)));
    ui->labelStake->setText(formatBitcoinAmountAsRichString(BitcoinUnits::formatWithUnit(unit, stake, true, false)));
    ui->labelUnconfirmed->setText(formatBitcoinAmountAsRichString(BitcoinUnits::formatWithUnit(unit, unconfirmedBalance, true, false)));
    ui->labelTotal->setText(formatBitcoinAmountAsRichString(BitcoinUnits::formatWithUnit(unit, balance + stake + unconfirmedBalance, true, false)));
}


void OverviewPage::setModel(WalletModel *model)
{
    this->model = model;
    if(model && model->getOptionsModel())
    {
        //LEAKLEAK
        RichTextDelegate* richDelegate = new RichTextDelegate(this);

        //LEAKLEAK
        QSortFilterProxyModel* sortableAccountModel = new QSortFilterProxyModel(this);
        sortableAccountModel->setSortRole(Qt::UserRole);
        sortableAccountModel->setSourceModel(model->getMyAccountModel());
        ui->PortfolioTable->setModel(sortableAccountModel);
        ui->PortfolioTable->setItemDelegateForColumn(AccountModel::Balance ,richDelegate);
        ui->PortfolioTable->setSortingEnabled(true);
        ui->PortfolioTable->sortByColumn(0);
        ui->PortfolioTable->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

        //LEAKLEAK
        SingleColumnAccountModel* listModel=new SingleColumnAccountModel(model->getMyAccountModel(), true, false, tr("Select account"));
        ui->quick_transfer_from_combobox->setModel(listModel);
        ui->quick_transfer_from_combobox->setItemDelegate(richDelegate);
        // Sadly the below is necessary in order to be able to style QComboBox pull down lists properly.
        ui->quick_transfer_from_combobox->setView(new QListView(this));

        //LEAKLEAK
        SingleColumnAccountModel* listModel2=new SingleColumnAccountModel(model->getExternalAccountModel(), false, false, tr("Select account"));
        ui->quick_transfer_to_combobox->setModel(listModel2);
        ui->quick_transfer_to_combobox->setItemDelegate(richDelegate);
        // Sadly the below is necessary in order to be able to style QComboBox pull down lists properly.
        ui->quick_transfer_to_combobox->setView(new QListView(this));

        // Keep up to date with wallet
        setBalance(model->getBalance(), model->getStake(), model->getUnconfirmedBalance(), model->getImmatureBalance());
        connect(model, SIGNAL(balanceChanged(qint64, qint64, qint64, qint64)), this, SLOT(setBalance(qint64, qint64, qint64, qint64)));

        connect(model->getOptionsModel(), SIGNAL(displayUnitChanged(int)), this, SLOT(updateDisplayUnit()));
    }

    // update the display unit, to not use the default ("BTC")
    updateDisplayUnit();
}

void OverviewPage::updateDisplayUnit()
{
    if(model && model->getOptionsModel())
    {
        int unit = model->getOptionsModel()->getDisplayUnit();

        if(currentBalance != -1)
            setBalance(currentBalance, model->getStake(), currentUnconfirmedBalance, currentImmatureBalance);

        ui->quick_transfer_amount->setDisplayUnit(unit);

        // Update txdelegate->unit with the current unit
        //txdelegate->unit = model->getOptionsModel()->getDisplayUnit();

        //ui->listTransactions->update();
    }
}

void OverviewPage::showOutOfSyncWarning(bool fShow)
{
    //ui->labelWalletStatus->setVisible(fShow);
    //ui->labelTransactionsStatus->setVisible(fShow);
}

void OverviewPage::on_quick_transfer_next_button_clicked()
{
    QList<SendCoinsRecipient> recipients;

    if(!model)
        return;

    if(!ui->quick_transfer_amount->validate())
    {
        ui->quick_transfer_amount->setValid(false);
        return;
    }
    else
    {
        if(ui->quick_transfer_amount->value() <= 0)
        {
            // Cannot send 0 coins or less
            ui->quick_transfer_amount->setValid(false);
            return;
        }
    }
    if(ui->quick_transfer_to_combobox->currentIndex() < 1)
    {
        //fixme: indicate invalid somehow
        //ui->quick_transfer_to_combobox->setValid(false);
        return;
    }
    if(ui->quick_transfer_from_combobox->currentIndex() < 1)
    {
        //fixme: indicate invalid somehow
        //ui->quick_transfer_from_combobox->setValid(false);
        return;
    }

    int toIndex = ui->quick_transfer_to_combobox->currentIndex() - 1;
    int fromIndex = ui->quick_transfer_from_combobox->currentIndex() - 1;
    QString toAddress = model->getAllAccountModel()->data(AccountModel::Address, toIndex).toString().trimmed();
    QString toLabel = model->getAllAccountModel()->data(AccountModel::Label, toIndex).toString().trimmed();
    QString fromAccountAddress = model->getMyAccountModel()->data(AccountModel::Address, fromIndex).toString().trimmed();

    qint64 amt=ui->quick_transfer_amount->value();

    recipients.append(SendCoinsRecipient(amt, toAddress, toLabel));

    if(GUIUtil::SendCoinsHelper(this, recipients, model, fromAccountAddress, true))
    {
        ui->quick_transfer_amount->clear();
        ui->quick_transfer_to_combobox->clear();
        ui->quick_transfer_from_combobox->clear();
    }
}

