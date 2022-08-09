#ifndef FITBITCHART_H
#define FITBITCHART_H

#include <QWidget>
#include <QChartView>
#include <QVBoxLayout>
#include <QDateTime>
#include <QSplineSeries>
#include <QDateTimeAxis>
#include <QValueAxis>

class FitbitChart : public QWidget
{
    Q_OBJECT
public:
    struct Activity {
        QString category;
        qreal value = 0;
        QDate date;
    };

    enum Duration {
        Day = 0,
        Week = 1,
        Month = 2,
        Year = 3
    };

    FitbitChart(QWidget *parent = nullptr);
    virtual ~FitbitChart();

    const QDate &startDate() const;
    void setStartDate(const QDate &newStartDate);

    const QDate &endDate() const;
    void setEndDate(const QDate &newEndDate);

    Duration duration() const;
    void setDuration(Duration newDuration);

public Q_SLOTS:
    void setActivities(const QHash<QString, QList<FitbitChart::Activity>> &activities);
    void reload();

Q_SIGNALS:

private:
    QtCharts::QChartView *mChart;

    struct SeriesType {
        QtCharts::QSplineSeries *series = Q_NULLPTR;
        qreal maxValue = 0;
        QDate minDate;
        QDate maxDate;
    };

    QHash<QString, SeriesType> mSeriesHash;

    QVBoxLayout *mLayout;
    Duration mDuration = Month;

    QHash<QString, QList<FitbitChart::Activity>> mActivities;

    QDate mStartDate;
    QDate mEndDate;

    QtCharts::QDateTimeAxis *mAxisX = Q_NULLPTR;
    QtCharts::QValueAxis *mAxisY = Q_NULLPTR;

protected:
    SeriesType addSeries(const QString &category, const QList<FitbitChart::Activity> &activities);
    void clearSeries();
    void removeSeries(const QString &category);
};

#endif // FITBITCHART_H
