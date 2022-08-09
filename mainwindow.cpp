#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFile>
#include <QFileDialog>
#include <QDebug>
#include <QLabel>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    mStartDate = new QDateEdit();
    mStartDate->setDisplayFormat("yyyy MMM dd");
    mStartDate->setCalendarPopup(true);
    mStartDate->setEnabled(false);

    mEndDate = new QDateEdit();
    mEndDate->setDisplayFormat("yyyy MMM dd");
    mEndDate->setCalendarPopup(true);
    mEndDate->setEnabled(false);

    mDuration = new QComboBox();
    mDuration->addItems({"Daily", "Weekly", "Monthly", "Yearly"});
    mDuration->setCurrentIndex(2);

    auto dateWidget = new QWidget;
    auto dateLayout = new QHBoxLayout(dateWidget);
    dateLayout->addStretch();
    dateLayout->addWidget(new QLabel("Duration:"));
    dateLayout->addWidget(mDuration);
    dateLayout->addWidget(new QLabel("From:"));
    dateLayout->addWidget(mStartDate);
    dateLayout->addWidget(new QLabel("To:"));
    dateLayout->addWidget(mEndDate);

    connect(mDuration, &QComboBox::textActivated, this, &MainWindow::reloadAll);
    connect(mStartDate, &QDateEdit::dateChanged, this, &MainWindow::reloadAll);
    connect(mEndDate, &QDateEdit::dateChanged, this, &MainWindow::reloadAll);

    ui->toolBar->addWidget(dateWidget);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_actionImport_Activities_triggered()
{
    auto list = QFileDialog::getOpenFileNames(this, tr("Import Activities"), QDir::homePath(), "CSV (*.csv)");

    mBlockReloading = true;

    QDate minDate = QDate::currentDate(), maxDate = QDate(1,1,1);

    QHash<TabTypes, QHash<QString, QMap<QDate, FitbitChart::Activity>>> data;
    for (const auto &path: list)
    {
        QFile f(path);
        if (!f.open(QFile::ReadOnly))
            continue;

        auto lines = QString::fromUtf8(f.readAll()).split('\n');
        if (lines.count() < 3)
            continue;

        lines.removeFirst();
        auto headers = lines.takeFirst().split(',');

        for (const auto &l: lines)
        {
            auto parts = l.split("\",\"");
            auto date_parts = QString(parts.at(0)).remove('\"').remove(',').split('-');
            if (date_parts.count() != 3)
                continue;

            auto date = QDate(date_parts.at(0).toInt(), date_parts.at(1).toInt(), date_parts.at(2).toInt());

            minDate = std::min(minDate, date);
            maxDate = std::max(maxDate, date);

            const auto count = parts.count();
            for (int i=1; i<count; i++)
            {
                const auto header = headers.value(i);
                TabTypes tabIndex;
                if (header == QStringLiteral("Calories Burned") || header == QStringLiteral("Activity Calories"))
                    tabIndex = TabCalories;
                else
                if (header == QStringLiteral("Steps"))
                    tabIndex = TabSteps;
                else
                if (header == QStringLiteral("Distance"))
                    tabIndex = TabDistance;
                else
                if (header == QStringLiteral("Minutes Lightly Active") || header == QStringLiteral("Minutes Fairly Active") || header == QStringLiteral("Minutes Very Active"))
                    tabIndex = TabActiveTime;
                else
                    continue;

                auto &activity = data[tabIndex][header][date];
                activity.date = date;
                activity.value += QString(parts.at(i)).remove('\"').remove(',').toDouble();
            }
        }
    }

    QHashIterator<TabTypes, QHash<QString, QMap<QDate, FitbitChart::Activity>>> i(data);
    while (i.hasNext())
    {
        i.next();

        QHash<QString, QList<FitbitChart::Activity>> activities;

        QHashIterator<QString, QMap<QDate, FitbitChart::Activity>> j(i.value());
        while (j.hasNext())
        {
            j.next();
            activities[j.key()] = j.value().values();
        }

        switch (static_cast<int>(i.key()))
        {
        case TabCalories:
            ui->caloriesChart->setActivities(activities);
            break;
        case TabSteps:
            ui->stepsChart->setActivities(activities);
            break;
        case TabDistance:
            ui->distanceChart->setActivities(activities);
            break;
        case TabActiveTime:
            ui->activeTimeChart->setActivities(activities);
            break;
        }
    }

    mStartDate->setDate(minDate);
    mStartDate->setEnabled(true);

    mEndDate->setDate(maxDate);
    mEndDate->setEnabled(true);

    mBlockReloading = false;

    reloadAll();
}

void MainWindow::reloadAll()
{
    if (mBlockReloading)
        return;

    for (auto c: {
            ui->caloriesChart,
            ui->stepsChart,
            ui->distanceChart,
            ui->activeTimeChart,
    }) {
        c->setStartDate(mStartDate->date());
        c->setEndDate(mEndDate->date());
        c->setDuration( static_cast<FitbitChart::Duration>(mDuration->currentIndex()) );
        c->reload();
    }
}

