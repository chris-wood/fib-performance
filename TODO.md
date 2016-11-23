# TODO

- Finish single BloomFilter code
- Add prefix bloom filter data structure (collection of blocks, single BloomFilter maps to block, and then BloomFilter for bits inside the block)
- Implement merged bloom filter data structure
    - compute the k hashes
    - for each index in the resultant string
        - concat bits from N BFs together
        - AND to running total
    - output AND result