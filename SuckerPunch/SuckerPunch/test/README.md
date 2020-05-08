SuckerPunchCodingTest
=====================

# Test Plan
The purpose of this readme is to explain some test scenarios and strategy
for the SuckerPunchCoding.
1. Functional Requirements
*  System should be able to create Queue | R1
*  System should be able to destroy Queue | R2
*  System should be able to enque | R3
*  System should be able to dequeue | R4
*  System should be able to support until 2047 bytes in queues | R5
*  System should support 15 queues with an average of 80 or so bytes in each queue | R6

The following test cases will be covered
1. Queue/Dequeue
    * Normal values like: 1, 2, 3
    * Bigger values like: 20, 40, 60
    * Zero values
    * Negative values
2. OutOfMemory * Illegal operation (Stress Testing)
    * System should be able to handle when the memory is almost full.
    * System should not be able to handle when memory is full for cases like:
      * uses no more than 2048 bytes to implement all byte queues, and
      * must support 15 queues with an average of 80 or so bytes in each queue.
3. Test Code Coverage
    * Implemented with gcov
4. Continuos Integration (CircleCI)
    * In this way, we make sure that any change into the code is being tested.

# Test Framework
1. Selected Google Test Framework
Reasons:
* Is portable: it doesn’t require exceptions or RTTI; it works around various bugs in various compilers and environments.
* Nonfatal assertions, for example "EXPECT", are very useful when you want to report many failures.
* Assertions are easy to handle.
* Good Documentation.
* Easy to handle Death Test.

# Configuration and Environment
* Install Google test (GNU/Linux - Debian based)
All this configuration was performed on Ubuntu

```
Local host: 5.4.0-29-generic #33-Ubuntu SMP Wed Apr 29 14:32:27 UTC 2020 x86_64 x86_64 x86_64 GNU/Linux
CI:
Docker Engine Version: 18.09.6
Kernel Version: Linux 034df3269b07 4.15.0-1052-aws #54-Ubuntu SMP Tue Oct 1 15:43:26 UTC 2019 x86_64 Linux
```

1. Install gtest development package and google-mock
```
$ sudo apt-get install libgtest-dev
$ sudo apt-get install -y google-mock
or as a best practice you can do the next

$ cd ~
$ git clone https://github.com/google/googletest.git
$ cd googletest
$ mkdir build && cd build
$ cmake .. -DBUILD_SHARED_LIBS=ON -DINSTALL_GTEST=ON -DCMAKE_INSTALL_PREFIX:PATH=/usr
$ make -j8
$ sudo make install
$ sudo ldconfig
```
2. Install cmake
```
$ sudo apt-get install cmake
```
3. Compile CMakeLists
```
$ cd /usr/src/gtest
$ sudo cmake CMakeLists.txt
$ sudo make
# copy or symlink "libgtest.a" and "libgtest_main.a: to your /usr/lib folder
$ sudo cp lib/*.a /usr/lib
```
4. Install gcov
`sudo apt-get install -y lcov`


# Run the tests
* Being in `test` directory and type the following commands
```
$ cmake CMakeLists.txt
$ make
$ ./runTests
XML Test report, just add the following flag
$ ./runTests --gtest_output=xml
```

# Test Code Coverage
* Being in `test` directory
```
$ gcovr -r . # To generate test summary
$ gcovr -r . --branches # To generate tabular output with the number of branches
$ gcovr -r . --xml-pretty # XML Report (Useful on CI)
$ gcovr -r . --html --html-details -o test-report-detailed.html # HTML Report

```
