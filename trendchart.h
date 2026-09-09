#ifndef TRENDCHART_H
#define TRENDCHART_H

#include <QWidget>
#include <QVector>

/*
 * 自绘实时曲线控件(QPainter, 不依赖 QtCharts)。
 * 三条数据带: 温度(0~50℃) / 湿度(0~100%RH) / 光照(自动量程 lux)。
 * 数值以 NaN 作"本次无效"哨兵, 曲线在无效点处断开。
 */
class TrendChart : public QWidget
{
    Q_OBJECT
public:
    explicit TrendChart(QWidget *parent = 0);

    void addSample(double tempC, double humRH, int lux);
    void clearData();

protected:
    void paintEvent(QPaintEvent *e) override;

private:
    struct Band
    {
        const char *name;
        double      minV;
        double      maxV;      /* lux 带为 0 时按数据自动量程 */
        QColor      color;
        QVector<double> data;
        double      autoMax;   /* 自动量程的上限 */
    };

    static const int kMaxPoints = 180;

    Band m_bandT;   /* 温度 */
    Band m_bandH;   /* 湿度 */
    Band m_bandL;   /* 光照 */
};

#endif // TRENDCHART_H
