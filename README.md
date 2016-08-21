# external_sort
Concurrent external sort implementation for arbitrary type.

The only argument that is required for the tests to run is the absolute path to an existing folder WITH trailing slashes.
For example:
```
./external_sort /home/test_user/test/
start external_sort.exe C:\\test\\
```

During the testing the code will generate, depending on your system bitness, up to ~160 MB of data( up to ~80 MB takes the source file containing 10 000 000 size_t numbers, and approx the same size is required for the algorithm to proceed with it's execution).
