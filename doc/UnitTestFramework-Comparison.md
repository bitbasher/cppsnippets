# Unit Testing Framework Comparison

**Date**: December 20, 2025  
**Context**: Evaluating testing frameworks for OpenSCAD resource management classes

---

## 1. User Request

> We have been using both Google Test and Catch2 for unit testing our code, and writing special test apps to exercise the discovery code in a mock file system at testFileStructure.
>
> I have noticed that Qt also has a good looking test system, as described here: [Qt Test Overview](https://doc.qt.io/qt-6/qtest-overview.html)
>
> With examples for doing TDD work here: [Qt Test Tutorial](https://doc.qt.io/qt-6/qttestlib-tutorial2-example.html)
>
> Can you compare the three toolsets for me? I am favouring the Qt version but have not seen it in use. We had good results using Catch2 - it is easy to work with. I don't know much about Google Test.

---

## 2. Framework Overview

### 2.1 Google Test (gtest)

Google's C++ testing framework, widely used in industry. Requires linking a separate library.

**Website**: https://github.com/google/googletest

### 2.2 Catch2

Modern, header-only C++ test framework with BDD-style syntax. Known for simplicity and readable test output.

**Website**: https://github.com/catchorg/Catch2

### 2.3 Qt Test

Qt's built-in testing framework, tightly integrated with the Qt framework. Part of the Qt installation.

**Documentation**: https://doc.qt.io/qt-6/qtest-overview.html

---

## 3. Feature Comparison Matrix

| Feature | Google Test | Catch2 | Qt Test |
|---------|-------------|--------|---------|
| **Setup Complexity** | Medium (separate lib) | Low (header-only) | Low (part of Qt) |
| **Test Registration** | `TEST()` macro | `TEST_CASE()` macro | Private slots |
| **Data-Driven Tests** | Parameterized tests | `GENERATE()` | `_data()` functions |
| **GUI Testing** | ❌ Needs Qt Test | ❌ Needs Qt Test | ✅ Built-in |
| **Signal/Slot Testing** | ❌ | ❌ | ✅ `QSignalSpy` |
| **Benchmarking** | `--benchmark` flag | Chrono timing | ✅ `QBENCHMARK` |
| **Mocking** | ✅ Google Mock | Limited | ❌ |
| **IDE Integration** | ✅ Excellent | ✅ Good | ✅ Qt Creator native |
| **Output Formats** | XML, JSON | XML, JUnit, TAP | XML, JUnit, TAP, CSV, TeamCity |
| **Learning Curve** | Medium | Low | Low (if you know Qt) |
| **Thread Safety** | ✅ | ✅ | ✅ |
| **Header-Only** | ❌ | ✅ (v3) | ❌ (Qt dependency) |

---

## 4. Syntax Comparison

### 4.1 Google Test

```cpp
#include <gtest/gtest.h>

TEST(StringTest, ToUpper) {
    std::string s = "hello";
    // ... transform to upper
    EXPECT_EQ(s, "HELLO");
}

// Data-driven (Parameterized)
class ToUpperTest : public testing::TestWithParam<std::pair<std::string, std::string>> {};

TEST_P(ToUpperTest, Works) {
    auto [input, expected] = GetParam();
    EXPECT_EQ(toUpper(input), expected);
}

INSTANTIATE_TEST_SUITE_P(Strings, ToUpperTest, testing::Values(
    std::make_pair("hello", "HELLO"),
    std::make_pair("Hello", "HELLO")
));
```

**Characteristics**:
- Uses `TEST()` and `TEST_F()` macros
- Parameterized tests require class inheritance
- `EXPECT_*` for non-fatal, `ASSERT_*` for fatal assertions
- Verbose but powerful

### 4.2 Catch2

```cpp
#include <catch2/catch_test_macros.hpp>

TEST_CASE("String toUpper", "[string]") {
    SECTION("lowercase becomes uppercase") {
        REQUIRE(toUpper("hello") == "HELLO");
    }
    
    SECTION("mixed case becomes uppercase") {
        REQUIRE(toUpper("Hello") == "HELLO");
    }
}

// Data-driven with GENERATE
TEST_CASE("ToUpper data-driven") {
    auto [input, expected] = GENERATE(table<std::string, std::string>({
        {"hello", "HELLO"},
        {"Hello", "HELLO"},
        {"HELLO", "HELLO"}
    }));
    
    REQUIRE(toUpper(input) == expected);
}
```

**Characteristics**:
- Natural, readable syntax
- `SECTION` blocks for test organization
- `GENERATE()` for elegant data-driven tests
- Tags (`[string]`, `[slow]`) for filtering
- BDD-style `SCENARIO`/`GIVEN`/`WHEN`/`THEN` available

### 4.3 Qt Test

```cpp
#include <QTest>

class TestQString : public QObject {
    Q_OBJECT
    
private slots:
    void initTestCase() { qDebug() << "Starting tests"; }
    
    void toUpper_data() {
        QTest::addColumn<QString>("input");
        QTest::addColumn<QString>("expected");
        
        QTest::newRow("all-lower") << "hello" << "HELLO";
        QTest::newRow("mixed")     << "Hello" << "HELLO";
        QTest::newRow("all-upper") << "HELLO" << "HELLO";
    }
    
    void toUpper() {
        QFETCH(QString, input);
        QFETCH(QString, expected);
        QCOMPARE(input.toUpper(), expected);
    }
    
    void cleanupTestCase() { qDebug() << "Tests complete"; }
};

QTEST_MAIN(TestQString)
#include "testqstring.moc"
```

**Characteristics**:
- Test class inherits from `QObject`
- Test functions are private slots
- `_data()` suffix for data-driven tests
- `QFETCH()` retrieves test data
- Requires MOC processing

---

## 5. Feature Deep Dive

### 5.1 Data-Driven Testing

| Framework | Approach | Verbosity | Flexibility |
|-----------|----------|-----------|-------------|
| **Google Test** | `TestWithParam<T>` + `INSTANTIATE_TEST_SUITE_P` | High | High |
| **Catch2** | `GENERATE(table<...>({...}))` | Low | Medium |
| **Qt Test** | `testName_data()` + `QFETCH()` | Medium | High |

**Winner**: Catch2 for simplicity, Qt Test for Qt type integration

### 5.2 GUI/Widget Testing

| Framework | Support | Example |
|-----------|---------|---------|
| **Google Test** | ❌ None | N/A |
| **Catch2** | ❌ None | N/A |
| **Qt Test** | ✅ Built-in | `QTest::mouseClick()`, `QTest::keyClick()` |

**Example (Qt Test only)**:
```cpp
void TestWidget::testButtonClick() {
    MyWidget widget;
    widget.show();
    
    QTest::mouseClick(widget.button(), Qt::LeftButton);
    
    QCOMPARE(widget.clickCount(), 1);
}
```

**Winner**: Qt Test (exclusive feature)

### 5.3 Signal/Slot Testing

**Only available in Qt Test**:
```cpp
void TestWidget::testButtonEmitsSignal() {
    MyWidget widget;
    QSignalSpy spy(&widget, &MyWidget::buttonClicked);
    
    QTest::mouseClick(widget.button(), Qt::LeftButton);
    
    QCOMPARE(spy.count(), 1);
    QCOMPARE(spy.takeFirst().at(0).toString(), "expected value");
}
```

**Winner**: Qt Test (exclusive feature)

### 5.4 Benchmarking

| Framework | Approach | Measurement Options |
|-----------|----------|---------------------|
| **Google Test** | `--benchmark_filter` flag | Basic timing |
| **Catch2** | `BENCHMARK` macro | Chrono-based |
| **Qt Test** | `QBENCHMARK` | Walltime, CPU ticks, Valgrind, Linux perf |

**Qt Test Example**:
```cpp
void TestScanner::benchmarkScan() {
    ResourceScanner scanner;
    QBENCHMARK {
        scanner.scanLocation("/path/to/resources");
    }
}
```

**Command-line options**:
- `-callgrind` - Use Valgrind Callgrind
- `-perf` - Use Linux perf events
- `-tickcounter` - Use CPU tick counters

**Winner**: Qt Test (most measurement backends)

### 5.5 Mocking

| Framework | Built-in Mocking | External Options |
|-----------|------------------|------------------|
| **Google Test** | ✅ Google Mock | N/A |
| **Catch2** | ❌ | FakeIt, Trompeloeil |
| **Qt Test** | ❌ | Same external options |

**Winner**: Google Test (Google Mock is excellent)

---

## 6. Qt Test Special Features

### 6.1 Test Lifecycle Hooks

```cpp
private slots:
    void initTestCase();      // Before first test
    void initTestCase_data(); // Global test data
    void cleanupTestCase();   // After last test
    void init();              // Before each test
    void cleanup();           // After each test
```

### 6.2 Output Formats

```bash
./mytest -o results.xml,xml      # XML output
./mytest -o results.xml,junitxml # JUnit XML
./mytest -o results.tap,tap      # Test Anything Protocol
./mytest -teamcity               # TeamCity format
./mytest -csv                    # CSV (benchmarks)
```

### 6.3 Command-Line Filtering

```bash
./mytest toUpper                 # Run only toUpper test
./mytest toUpper:mixed           # Run only "mixed" data row
./mytest -v2                     # Verbose output
./mytest -vs                     # Show all signals
./mytest -platform offscreen     # Headless GUI testing
```

### 6.4 Key Macros

| Macro | Purpose |
|-------|---------|
| `QVERIFY(condition)` | Assert condition is true |
| `QVERIFY2(condition, message)` | Assert with custom message |
| `QCOMPARE(actual, expected)` | Assert equality with diff |
| `QFETCH(Type, name)` | Get data from `_data()` table |
| `QFETCH_GLOBAL(Type, name)` | Get global test data |
| `QBENCHMARK { ... }` | Measure code block |
| `QSKIP("reason")` | Skip test with message |
| `QEXPECT_FAIL(tag, msg, mode)` | Mark expected failure |
| `QTEST_MAIN(TestClass)` | Generate main() |

---

## 7. CMake Integration

### 7.1 Google Test

```cmake
find_package(GTest REQUIRED)

add_executable(my_tests test.cpp)
target_link_libraries(my_tests GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(my_tests)
```

### 7.2 Catch2

```cmake
find_package(Catch2 REQUIRED)

add_executable(my_tests test.cpp)
target_link_libraries(my_tests Catch2::Catch2WithMain)

include(Catch)
catch_discover_tests(my_tests)
```

### 7.3 Qt Test

```cmake
find_package(Qt6 REQUIRED COMPONENTS Test)

qt_add_executable(my_tests test.cpp)
target_link_libraries(my_tests PRIVATE Qt6::Test)

enable_testing()
add_test(NAME MyTests COMMAND my_tests)
```

---

## 8. Pros and Cons Summary

### 8.1 Google Test

**Pros**:
- Industry standard, widely documented
- Google Mock for comprehensive mocking
- Death tests (verify crashes)
- Strong parameterized test support
- Works with any C++ project

**Cons**:
- Requires linking a library
- Verbose data-driven syntax
- No Qt-specific support
- Steeper learning curve

### 8.2 Catch2

**Pros**:
- Header-only (simple setup)
- Natural, readable syntax
- Elegant `SECTION` nesting
- `GENERATE()` for data-driven tests
- Excellent failure messages
- Tags for test filtering

**Cons**:
- No built-in mocking
- No Qt-specific support
- Slower compilation (header-only)

### 8.3 Qt Test

**Pros**:
- Native Qt integration
- GUI testing (`mouseClick`, `keyClick`)
- Signal spying (`QSignalSpy`)
- Built-in benchmarking with multiple backends
- Clean data-driven pattern
- Qt Creator integration
- Multiple output formats
- Thread-safe error reporting
- Already a dependency (using Qt anyway)

**Cons**:
- Only for Qt projects
- No mocking framework
- Less flexible than Catch2 sections
- One test class per executable (typically)

---

## 9. Recommendation

### 9.1 For This Project

Given that:
1. We're building a **Qt 6.10.1 application**
2. We need to test **resource discovery** with filesystem operations
3. We'll be testing **Qt Model/View** components (`ResourceModel`)
4. Future tests will involve **signals/slots** and potentially **widgets**

**Recommendation: Adopt Qt Test as the primary framework**

| Requirement | Qt Test Fit |
|-------------|-------------|
| Test `ResourceModel` (QAbstractItemModel) | ✅ Native `QModelIndex`, `QVariant` |
| Test signal emissions | ✅ `QSignalSpy` |
| Test with mock filesystem | ✅ `QTemporaryDir`, `QFile` |
| Data-driven tests for resource types | ✅ `_data()` pattern |
| Benchmark scanner performance | ✅ `QBENCHMARK` with perf |
| IDE integration | ✅ Qt Creator native |
| CMake/CTest integration | ✅ Built-in support |

### 9.2 Migration Strategy

1. **Keep existing tests** - Google Test and Catch2 tests continue to work
2. **New tests in Qt Test** - All new resource classes use Qt Test
3. **Gradual migration** - Convert existing tests as files are touched

### 9.3 Hybrid Configuration

All three frameworks can coexist:

```cmake
enable_testing()

# Google Test
add_executable(gtest_tests ...)
target_link_libraries(gtest_tests GTest::gtest_main)
add_test(NAME GTestSuite COMMAND gtest_tests)

# Catch2
add_executable(catch2_tests ...)
target_link_libraries(catch2_tests Catch2::Catch2WithMain)
add_test(NAME Catch2Suite COMMAND catch2_tests)

# Qt Test
qt_add_executable(qtest_tests ...)
target_link_libraries(qtest_tests Qt6::Test)
add_test(NAME QtTestSuite COMMAND qtest_tests)
```

---

## 10. References

- [Qt Test Overview](https://doc.qt.io/qt-6/qtest-overview.html)
- [Qt Test Tutorial](https://doc.qt.io/qt-6/qtest-tutorial.html)
- [Qt Test Best Practices](https://doc.qt.io/qt-6/qttest-best-practices-qdoc.html)
- [Google Test Documentation](https://google.github.io/googletest/)
- [Catch2 Documentation](https://github.com/catchorg/Catch2/blob/devel/docs/Readme.md)
