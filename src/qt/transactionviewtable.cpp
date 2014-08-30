#include "transactionviewtable.h"
#include "transactiontablemodel.h"

#include <QResizeEvent>
#include <QHeaderView>
#include <QVector>
#include <numeric>

TransactionViewTable::TransactionViewTable(QWidget *parent)
: QTableView(parent)
{

    setSelectionBehavior(QAbstractItemView::SelectItems);
    setSelectionMode(QAbstractItemView::NoSelection);
    setSortingEnabled(true);
    sortByColumn(TransactionTableModel::Date, Qt::DescendingOrder);
    verticalHeader()->hide();
    setShowGrid(false);

    // Turn manual resize grab handles off.
    horizontalHeader()->setResizeMode(QHeaderView::Fixed);
}

void TransactionViewTable::setModel(QAbstractItemModel *model)
{
    QTableView::setModel(model);
    connect(model, SIGNAL(layoutChanged()), this, SLOT(updateColumns()));
}

void TransactionViewTable::updateColumns()
{
    int columnCount = model()->columnCount();
    if(columnCount == 0)
        return;

    // fixme: hardcoded
    int totalWidthToFill = size().width()-20;

    QVector<int> columnWidths;
    for(int i = 0; i < columnCount; i++)
    {
        columnWidths.push_back(0);
    }

    // First three columns are special always allow them a fixed width.
    columnWidths[0] = 25;
    columnWidths[1] = 120;
    columnWidths[2] = 145;


    setColumnWidth(0, columnWidths[0]);
    setColumnWidth(1, columnWidths[1]);
    setColumnWidth(2, columnWidths[2]);

    int remWidth = (totalWidthToFill - std::accumulate(columnWidths.begin(),columnWidths.begin()+3,0)) / (columnCount - 3);
    for(int i = 3; i < columnCount; i++)
    {
        setColumnWidth(i, remWidth);
    }
    return;}

void TransactionViewTable::resizeEvent(QResizeEvent *event)
{
    QTableView::resizeEvent(event);
    updateColumns();
}
