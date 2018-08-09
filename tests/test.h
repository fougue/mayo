#pragma once

#include <QtCore/QObject>
#include <QtTest/QtTest>

namespace Mayo {

class Test : public QObject {
    Q_OBJECT

private slots:
    void CafUtils_test();
    void Quantity_test();
    void UnitSystem_test();

    void LibTree_test();
};

} // namespace Mayo
