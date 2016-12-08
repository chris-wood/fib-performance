# TODO

- XOR-based name hashing scheme (if names are not already hashed)
- Implement merged bloom filter data structure
    - compute the k hashes
    - for each index in the resultant string
        - concat bits from N BFs together
        - AND to running total
    - output AND result
- compact array hash table