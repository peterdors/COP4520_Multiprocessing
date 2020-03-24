This will hold the sequential implementations of our Linked List data structure.

To compile the C++ implementation from your terminal or command line, run the
run.sh bash file by making the following call: 

bash run.sh

This program was ran and tested using a MacOS Mojave version 10.14.6 and using the g++
compiler clang version

NOTES
=====
For function calls to Insert(T data, int thread_id) and
Delete(T data, int thread_id) the thread_id value should be in the range [0,3].

Since the assignment says to only spawn 4 threads, I have only implemented it to
handle up to 4 threads.
