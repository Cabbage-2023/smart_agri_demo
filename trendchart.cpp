#include "trendchart.h"

#include <QPainter>
#include <QPainterPath>
#include <QtMath>
#include <algorithm>

TrendChart::TrendChart(QWidget *parent)
    : QWidget(parent)
{
    setMinimumHeight(110);
    m_bandT.name = "温度(℃)";
    m_bandT.minV = 0;   m_bandT.maxV = 50;   m_bandT.color = QColor(0xff, 0x8a, 0x65);
    m_bandH.name = "湿度(%RH)";
    m_bandH.minV = 0;   m_bandH.maxV = 100;  m_bandH.color = QColor(0x4f, 0xc3, 0xf7);
    m_bandL.name = "光照(lux)";
    m_bandL.minV = 0;   m_bandL.maxV = 0;    m_bandL.color = QColor(0xff, 0xd5, 0x4f);
}

void TrendChart::addSample(double tempC, double humRH, int lux)
{
    m_bandT.data.push_back(qIsFinite(tempC) ? tempC : qQNaN());
    m_bandH.data.push_back(qIsFinite(humRH) ? humRH : qQNaN());
    m_bandL.data.push_back(lux >= 0 ? (double)lux : qQNaN());

    if (m_bandT.data.size() > kMaxPoints)
        m_bandT.data.remove(0, m_bandT.data.size() - kMaxPoints);
    if (m_bandH.data.size() > kMaxPoints)
        m_bandH.data.remove(0, m_bandH.data.size() - kMaxPoints);
    if (m_bandL.data.size() > kMaxPoints)
        m_bandL.data.remove(0, m_bandL.data.size() - kMaxPoints);

    /* 光照自动量程: 按缓冲内最大值留 15% 余量 */
    double mx = 1.0;
    for (int i = 0; i < m_bandL.data.size(); ++i)
    {
        if (qIsFinite(m_bandL.data.at(i)))
            mx = qMax(mx, m_bandL.data.at(i));
    }
    m_bandL.autoMax = mx * 1.15;

    update();
}

void TrendChart::clearData()
{
    m_bandT.data.clear();
    m_bandH.data.clear();
    m_bandL.data.clear();
    update();
}

void TrendChart::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);

    const QColor bg(0x11, 0x15, 0x1c);
    const QColor grid(0x26, 0x30, 0x41);
    const QColor text(0x7f, 0x8e, 0xa3);
    p.fillRect(rect(), bg);

    const int topPad = 2;
    const double gap = 4.0;
    const double bandH = (height() - topPad - gap * 2.0) / 3.0;
    const double left = 6.0;    /* 右侧留 46px 画标签 */
    const double right = width() - 52.0;

    Band *bands[3] = { &m_bandT, &m_bandH, &m_bandL };

    p.setFont(QFont("sans-serif", 8));

    for (int bi = 0; bi < 3; ++bi)
    {
        Band &b = *bands[bi];
        double y0 = topPad + bi * (bandH + gap);
        QRectF area(left, y0, qMax(0.0, right - left), bandH);
        if (area.width() <= 0 || area.height() <= 0)
            continue;

        p.fillRect(area, QColor(0x0e, 0x11, 0x16));

        /* 网格 + 刻度 */
        double vMax = b.maxV > 0 ? b.maxV : b.autoMax;
        if (vMax <= 0)
            vMax = 1;
        p.setPen(grid);
        for (int g = 0; g <= 2; ++g)
        {
            double yy = area.top() + area.height() * g / 2.0;
            p.drawLine(QPointF(area.left(), yy), QPointF(area.right(), yy));
            double vv = b.maxV > 0 ? (b.minV + (b.maxV - b.minV) * (1.0 - g / 2.0))
                                   : vMax * (1.0 - g / 2.0);
            p.setPen(text);
            p.drawText(QRectF(area.right() + 4, yy - 6, 44, 12),
                       Qt::AlignLeft | Qt::AlignVCenter,
                       QString::number(vv, 'g', 3));
            p.setPen(grid);
        }

        /* 曲线 */
        if (b.data.size() > 1)
        {
            double spanX = area.width() / (double)(kMaxPoints - 1);
            double vlo = b.minV, vhi = b.maxV > 0 ? b.maxV : vMax;
            double spanV = vhi - vlo;
            if (spanV <= 0)
                spanV = 1;

            QPainterPath path;
            bool penDown = false;
            int startIdx = b.data.size() > kMaxPoints ? b.data.size() - kMaxPoints : 0;
            for (int i = startIdx; i < b.data.size(); ++i)
            {
                double v = b.data.at(i);
                if (!qIsFinite(v))
                {
                    penDown = false;
                    continue;
                }
                double yy = area.bottom() - (v - vlo) / spanV * area.height();
                double xx = area.left() + (i - startIdx) * spanX;
                if (!penDown)
                {
                    path.moveTo(QPointF(xx, yy));
                    penDown = true;
                }
                else
                {
                    path.lineTo(QPointF(xx, yy));
                }
            }

            QPen lp(b.color, 1.6);
            p.setPen(lp);
            p.drawPath(path);
        }

        /* 带名标签 */
        p.setPen(text);
        p.setFont(QFont("sans-serif", 8));
        p.drawText(QRectF(area.left() + 3, area.top() + 1, 90, 12),
                   Qt::AlignLeft | Qt::AlignTop,
                   QString::fromUtf8(b.name));
    }
}
