#include "LearningChart.h"
#include <QPainter>
#include <QPaintEvent>
#include <QtGlobal>
#include <QPalette>
#include <QFont>
#include <QPen>
#include <QBrush>
#include <algorithm>

LearningChart::LearningChart(QWidget *parent) : QWidget(parent) {
    setFixedHeight(200);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
}

void LearningChart::setData(const QMap<QString, int>& data) {
    m_data = data;
    update();
}

void LearningChart::paintEvent(QPaintEvent *event) {
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    

    painter.fillRect(rect(), palette().base());
    
    if (m_data.isEmpty()) {
        painter.setPen(Qt::black);
        painter.drawText(rect(), Qt::AlignCenter, QStringLiteral("暂无复习数据"));
        return;
    }

    int margin = 40;
    int w = width() - 2 * margin;
    int h = height() - 2 * margin;
    

    int maxVal = 0;
    for (int val : m_data) {
        if (val > maxVal) maxVal = val;
    }
    if (maxVal == 0) maxVal = 10; 
    

    painter.setPen(QPen(palette().mid(), 2));
    painter.drawLine(margin, height() - margin, width() - margin, height() - margin);
    painter.drawLine(margin, margin, margin, height() - margin);
    

    QList<QString> keys = m_data.keys();

    
    int stepX = w / (keys.count() > 1 ? keys.count() - 1 : 1);
    
    painter.setPen(QPen(QColor("#3B82F6"), 3));
    
    int prevX = 0;
    int prevY = 0;

    for (int i = 0; i < keys.count(); ++i) {
        int val = m_data[keys[i]];
        int x = margin + i * stepX;
        int y = height() - margin - (val * h / maxVal);
        
        if (i > 0) {
            painter.drawLine(prevX, prevY, x, y);
        }
        
    
        painter.setBrush(QColor("#3B82F6"));
        painter.drawEllipse(QPoint(x, y), 5, 5);
        
    
        painter.setPen(QPen(palette().text(), 1));
        painter.drawText(QRect(x - 25, height() - margin + 5, 50, 20), Qt::AlignCenter, keys[i].right(5)); 
        
    
        if (val > 0) {
            painter.drawText(QRect(x - 20, y - 25, 40, 20), Qt::AlignCenter, QString::number(val));
        }
        
        painter.setPen(QPen(QColor("#3B82F6"), 3));
        
        prevX = x;
        prevY = y;
    }
    

    painter.setPen(QPen(palette().text(), 1));
    painter.setFont(QFont("Segoe UI", 12, QFont::Bold));
    painter.drawText(rect().adjusted(0, 10, 0, 0), Qt::AlignTop | Qt::AlignHCenter, QStringLiteral("最近7天复习趋势"));
}
