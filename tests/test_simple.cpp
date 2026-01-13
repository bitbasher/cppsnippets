#include <QtTest/QtTest>
#include <QDebug>

class SimpleTest : public QObject {
    Q_OBJECT
    
private slots:
    void initTestCase() {
        qDebug() << "SimpleTest: initTestCase called";
    }
    
    void testBasic() {
        qDebug() << "SimpleTest: testBasic running";
        QVERIFY(true);
    }
};

QTEST_MAIN(SimpleTest)
#include "test_simple.moc"
