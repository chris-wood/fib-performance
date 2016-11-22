# FIB hashing experiment

We want to test the performance cost or gain from using readable or
hashed names. The experiment works as follows:

Input:
    - A list of CCNx names L
    - A number n for the longest prefix to insert into the FIB

Procedure:
    - Prepopulate the FIB with every name up to n components
    - For each name N in L:
        - Measure the time to lookup N
    - Compute some statistics for the times


Goal:
- Support different FIB algorithms
- Support parallel and sequential lookup algorithms

Algorithms:
- Naive hash table [done]
- (hash table - Cisco) So et al. [done]
- (patricia trie with name compression) Song et al. [in progress]
- (bloom filter with hash table - Caesar) Perino et al. [in progress]
- (merged bloom filter) Dong et al. [in progress]
