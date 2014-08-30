#ifndef RICHTEXTDELEGATE_H
#define RICHTEXTDELEGATE_H

#include <QStyledItemDelegate>
#include <QString>

class QLabel;

// Helper function to handle currency formatting (smaller numbers after decimal point etc. - used in various other parts of the code)
QString formatBitcoinAmountAsRichString(QString stringIn);


// Delegate so that QTableView, QComboBox, etc. can handle rich text.
class RichTextDelegate : public QStyledItemDelegate
{
    Q_OBJECT
public:
    explicit RichTextDelegate(QObject *parent = 0);
    ~RichTextDelegate();
    void paint ( QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
    QSize sizeHint ( const QStyleOptionViewItem & option, const QModelIndex & index ) const;

signals:

public slots:

private:
    QLabel* renderControl;

};

#endif // RICHTEXTDELEGATE_H
