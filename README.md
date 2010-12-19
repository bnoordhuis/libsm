# libsm

A fast string matcher library.

## Goals

1. Should be able to match one or more strings against the input text.

2. Should be able to find one ore more strings in the input text.

This is different from e.g. `strcmp` and `strstr` because those functions
will only let you match against or search for a single string.

Now imagine you want to match against or search for a million strings...

## Status

Pre-alpha work-in-progress.

## To do

Lots.

* Suffix tree. Current implementation is O(n^2) space and O(n^3) time. Switch to Ukkonen's algorithm, it's O(n) for both.

* Pattern matcher. Model as a DFA. Use longest substring finding to keep the number of states down (this is where the suffix
  tree comes into play).

* Pattern matcher. Should be able to both pre-generate the matcher (serialized, or as C code) and to dynamically generate it
  at run-time. Fairly trivial, just the SMOP.
