#include "tabbar.h"
#include <QPainter>
#include <QTabBar>

TabBar::TabBar(QWidget *parent)
: QTabWidget(parent)
{
}

TabBar::~TabBar()
{
}

const int logoVertPadding=6;
const int logoHorPadding=25;
void TabBar::paintEvent(QPaintEvent* event)
{
    // -2px because pane overlaps by 2px.
    const int tabBarHeight=QTabWidget::tabBar()->height()-2;

    // Scale image to tab bar height and draw it (vertically centered) over the left of the tab bar with a bit of padding.
    QImage image(":images/toolbar_banner");
    QPainter paint(this);
    paint.setBrush(QBrush(QColor(235,235,235,255)));
    paint.setPen(QPen(QColor(235,235,235,255)));
    paint.drawRect(0,0,width(),tabBarHeight);
    paint.drawImage(logoHorPadding,logoVertPadding,image.scaledToHeight(tabBarHeight-(logoVertPadding*2)));

    // Draw a bottom border for tab bar (selected tab will draw over this border)
    paint.setPen(QPen(QColor("#DCDDDF")));
    paint.setBrush(QBrush(QColor("#DCDDDF")));
    paint.drawRect(0,tabBarHeight-2,width(),tabBarHeight);
    paint.end();
}
