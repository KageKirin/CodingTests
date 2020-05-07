SuckerPunchCodingTest
=====================

# Test Plan
The purpose of this readme is to explain some test scenarios and strategy
for the SuckerPunchCoding.

The following test cases will be covered
1. Queue/Dequeue
    a) Normal values like: 1, 2, 3
    b) Bigger values like: 20, 40, 60
    c) Zero values
    d) Negative values
2. OutOfMemory * Illegal operation
    a) System should be able to handle when the memory is almost full
    b) System should not be able to handle when memory is full for cases like:
      * uses no more than 2048 bytes to implement all byte queues, and
      * must support 15 queues with an average of 80 or so bytes in each queue.

# Test Framework
1. Selected Google Test Framework
Reasons:
* Is portable: it doesn’t require exceptions or RTTI; it works around various bugs in various compilers and environments.
* Nonfatal assertions, for example "EXPECT", are very useful when you want to report many failures.
* Assertions are easy to handle.
* Good Documentation.
* Easy to handle Death Test.

# Configuration
* Install Google test
If having Debian base GNU/Linux
1. Install gtest development package
```
$ sudo apt-get install libgtest-dev
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
sudo cp *.a /usr/lib
```
# Run the tests
* Go to `test` directory and type the following commands
```
$ cmake CMakeLists.txt
$ make
$ ./runTests
```

