# TODO

- Finish single BF code
- Add prefix bloom filter data structure (collection of blocks, single BF maps to block, and then BF for bits inside the block)
- Implement merged bloom filter data structure
    - compute the k hashes
    - for each index in the resultant string
        - concat bits from N BFs together
        - AND to running total
    - output AND result