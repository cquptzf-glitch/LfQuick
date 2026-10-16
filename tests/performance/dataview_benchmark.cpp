#include <LfQuick/DataView/DataFilterProxyModel.h>
#include <LfQuick/DataView/DataTableModel.h>

#include <QCoreApplication>
#include <QElapsedTimer>
#include <QTextStream>
#include <QVariantList>

#include <array>
#include <functional>

using namespace LfQuick;

namespace {

QStringList makeKeys(int columnCount)
{
    QStringList keys;
    keys.reserve(columnCount);
    for (int column = 0; column < columnCount; ++column)
        keys.push_back(QStringLiteral("c%1").arg(column));
    return keys;
}

QVariantList makeRows(int rowCount, const QStringList &keys)
{
    QVariantList rows;
    rows.reserve(rowCount);
    for (int row = 0; row < rowCount; ++row) {
        QVariantMap values;
        for (int column = 0; column < keys.size(); ++column) {
            if (column == 0)
                values.insert(keys.at(column), QStringLiteral("R-%1").arg(row, 6, 10, QLatin1Char('0')));
            else if (column == 1)
                values.insert(keys.at(column), row % 7 == 0 ? QStringLiteral("Review") : QStringLiteral("Ready"));
            else
                values.insert(keys.at(column), (row * 17 + column * 31) % 100000);
        }
        rows.push_back(values);
    }
    return rows;
}

qint64 elapsedFor(const std::function<void()> &work)
{
    QElapsedTimer timer;
    timer.start();
    work();
    return timer.elapsed();
}

struct Metrics {
    int rows = 0;
    int columns = 0;
    qint64 populateMs = 0;
    qint64 filterMs = 0;
    qint64 sortMs = 0;
    std::array<qint64, 3> updateBatchMs{};
};

Metrics runCase(int rowCount, int columnCount)
{
    const QStringList keys = makeKeys(columnCount);
    const QVariantList rows = makeRows(rowCount, keys);

    DataTableModel model;
    model.setColumnKeys(keys);

    Metrics metrics;
    metrics.rows = rowCount;
    metrics.columns = columnCount;
    metrics.populateMs = elapsedFor([&] { model.setRows(rows); });

    DataFilterProxyModel proxy;
    proxy.setSourceModel(&model);
    metrics.filterMs = elapsedFor([&] {
        proxy.setFilterText(QStringLiteral("Review"));
        const int forcedCount = proxy.rowCount();
        Q_UNUSED(forcedCount);
    });

    metrics.sortMs = elapsedFor([&] {
        model.sortByKey(keys.last(), Qt::DescendingOrder);
        const int forcedCount = proxy.rowCount();
        Q_UNUSED(forcedCount);
    });

    constexpr std::array<int, 3> updateRates{1, 10, 100};
    for (int rateIndex = 0; rateIndex < static_cast<int>(updateRates.size()); ++rateIndex) {
        const int batchSize = updateRates.at(rateIndex);
        metrics.updateBatchMs[rateIndex] = elapsedFor([&] {
            for (int update = 0; update < batchSize; ++update) {
                const int row = (update * 9973) % rowCount;
                model.setCell(row, keys.at(qMin(2, keys.size() - 1)), update);
            }
            const int forcedCount = proxy.rowCount();
            Q_UNUSED(forcedCount);
        });
    }

    return metrics;
}

} // namespace

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QTextStream out(stdout);

    out << "LfQuick DataView benchmark\n";
    out << "rows,columns,populate_ms,filter_ms,sort_ms,update_1_ms,update_10_ms,update_100_ms\n";

    constexpr std::array<int, 2> rowCounts{20000, 50000};
    constexpr std::array<int, 3> columnCounts{8, 20, 40};

    for (int rows : rowCounts) {
        for (int columns : columnCounts) {
            const Metrics metrics = runCase(rows, columns);
            out << metrics.rows << ','
                << metrics.columns << ','
                << metrics.populateMs << ','
                << metrics.filterMs << ','
                << metrics.sortMs << ','
                << metrics.updateBatchMs[0] << ','
                << metrics.updateBatchMs[1] << ','
                << metrics.updateBatchMs[2] << '\n';
            out.flush();
        }
    }

    out << "Note: update columns report one synthetic batch containing 1/10/100 model updates. "
           "No absolute performance threshold is enforced; use these metrics for regression tracking.\n";
    return 0;
}
