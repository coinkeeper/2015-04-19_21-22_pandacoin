#ifndef PORTFOLIOOVERVIEWTABLE_H
#define PORTFOLIOOVERVIEWTABLE_H

#include <QTableView>

class QResizeEvent;

class PortfolioOverviewTable : public QTableView
{
    Q_OBJECT
public:
    explicit PortfolioOverviewTable(QWidget *parent = 0);
    virtual void resizeEvent(QResizeEvent *event);
signals:

public slots:

};

#endif // PORTFOLIOOVERVIEWTABLE_H
