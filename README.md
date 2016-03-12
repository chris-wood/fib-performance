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