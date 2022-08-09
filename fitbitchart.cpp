#include "fitbitchart.h"

#include <QtMath>
#include <QDebug>

using namespace QtCharts;

FitbitChart::FitbitChart(QWidget *parent)
    : QWidget(parent)
{
    mChart = new QChartView;

    auto chart = mChart->chart();
    chart->setBackgroundBrush(palette().window());

    mLayout = new QVBoxLayout(this);
    mLayout->addWidget(mChart);
}

FitbitChart::~FitbitChart()
{

}

void FitbitChart::setActivities(const QHash<QString, QList<Activity> > &activities)
{
    mActivities = activities;
}

void FitbitChart::reload()
{
    clearSeries();

    qreal maxValue = 0;
    QDate minDate = QDate::currentDate();
    QDate maxDate = QDate(1,1,1);

    QHashIterator<QString, QList<FitbitChart::Activity>> i(mActivities);
    while (i.hasNext())
    {
        i.next();
        auto s = addSeries(i.key(), i.value());
        maxValue = std::max(maxValue, s.maxValue);
        minDate = std::min(minDate, s.minDate);
        maxDate = std::max(maxDate, s.maxDate);
    }

    auto chart = mChart->chart();

    mAxisY = new QValueAxis(this);
    mAxisY->setMin(0);

    int maxValueLogPow = pow(10, floor(log10(maxValue)));

    mAxisY->setMax( (1 + floor(maxValue/maxValueLogPow)) * maxValueLogPow );

    mAxisX = new QDateTimeAxis(this);
    mAxisX->setFormat("yy MMM");
    mAxisX->setMin(QDateTime(minDate, QTime(0,0,0)));
    mAxisX->setMax(QDateTime(maxDate, QTime(0,0,0)));
    mAxisX->setTickCount( std::min<int>(10, minDate.daysTo(maxDate) / 30));

    for (auto s: mSeriesHash.values())
    {
        chart->addSeries(s.series);
        chart->setAxisX(mAxisX, s.series);
        chart->setAxisY(mAxisY, s.series);
    }
}

FitbitChart::SeriesType FitbitChart::addSeries(const QString &category, const QList<FitbitChart::Activity> &activities)
{
    removeSeries(category);

    std::map<QDate, qreal> durationMap;
    const auto currentDate = QDate::currentDate();
    const auto add_days = currentDate.daysInYear() - currentDate.dayOfYear();
    for (const auto &a: activities)
    {
        if (a.date < mStartDate)
            continue;
        if (a.date > mEndDate)
            continue;

        switch (static_cast<int>(mDuration))
        {
        case Day:
            durationMap[a.date] += a.value;
            break;
        case Week:
        {
            auto dt = QDate::fromJulianDay(std::floor(a.date.toJulianDay() / 7) * 7 + 7);
            durationMap[dt] += a.value;
        }
            break;
        case Month:
            durationMap[QDate(a.date.year(), a.date.month(), 1).addMonths(1).addDays(-1)] += a.value;
            break;
        case Year:
            durationMap[QDate(a.date.addDays(add_days).year()+1, 1, 1).addDays(-1)] += a.value;
            break;
        }
    }

    FitbitChart::SeriesType s;
    s.maxValue = 0;
    s.minDate = QDate::currentDate();
    s.maxDate = QDate(1,1,1);

    s.series = new QSplineSeries(this);
    s.series->setName(category);

    for (const auto &[k, v]: durationMap)
    {
        s.minDate = std::min(s.minDate, k);
        s.maxDate = std::max(s.maxDate, k);
        s.maxValue = std::max(s.maxValue, v);
        s.series->append( QDateTime(k, QTime(0,0,0)).toMSecsSinceEpoch(), v );
    }

    mSeriesHash[category] = s;
    return s;
}

void FitbitChart::clearSeries()
{
    for (const auto &s: mSeriesHash.keys())
        removeSeries(s);

    auto chart = mChart->chart();
    if (mAxisX)
    {
        chart->removeAxis(mAxisX);
        delete mAxisX;
    }
    if (mAxisY)
    {
        chart->removeAxis(mAxisY);
        delete mAxisY;
    }
}

void FitbitChart::removeSeries(const QString &category)
{
    if (!mSeriesHash.contains(category))
        return;

    auto chart = mChart->chart();
    auto s = mSeriesHash.take(category);
    if (s.series)
    {
        chart->removeSeries(s.series);
        delete s.series;
    }
}

FitbitChart::Duration FitbitChart::duration() const
{
    return mDuration;
}

void FitbitChart::setDuration(Duration newDuration)
{
    mDuration = newDuration;
}

const QDate &FitbitChart::endDate() const
{
    return mEndDate;
}

void FitbitChart::setEndDate(const QDate &newEndDate)
{
    mEndDate = newEndDate;
}

const QDate &FitbitChart::startDate() const
{
    return mStartDate;
}

void FitbitChart::setStartDate(const QDate &newStartDate)
{
    mStartDate = newStartDate;
}
