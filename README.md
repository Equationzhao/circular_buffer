# Circle Buffer

## This is a tiny c++ implementation of circle buffer (Circular linked list based).

## Features

- iterator support (in process)
- constexpr support (in process)
- user-defined allocator support (in process)
- user-defined deleter support (in process)
- module support (in process)
- concept support (in process)
- ranges support (?)

## Support Modifiers/Member access/Operators

* push (in process)
* assign (in process)
* write
* read
* at
* operator []
* iterators :
  * iterator
  * const_iterator
  * circular_iterator
  * reverse_iterator (in process)
  * const_reverse_iterator (in process)
* swap
* sort (in process)
* <=>
* clear (in process)
* resize (in process)
* . . .

## TODO LIST for now

* [refactor size_t in operator +,-,+=,-= of iterator](https://github.com/Equationzhao/circle_buffer/blob/9699238615541f67483564f8ed3f7383c842c7bb/circular_buffer.h#L35)
* [// ? compare the T data it contains or  test  are they actually the "same object"](https://github.com/Equationzhao/circle_buffer/blob/9699238615541f67483564f8ed3f7383c842c7bb/circular_buffer.h#L85-L87)
* [BUG: StackOverFlow](https://github.com/Equationzhao/circle_buffer/blob/9699238615541f67483564f8ed3f7383c842c7bb/circular_buffer.h#L224)
* test Iterators 
