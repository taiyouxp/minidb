The seven modules we need in order to build <code>minidb</code>:
1. **Page and data file:** Fixed-size records on fixed-size pages.
2. **Page cache:** Pages in memory, dirty page marking, write to disk.
3. **B+ tree:** Search and insert with node splitting.
4. **Parser and catalog:** CREATE TABLE, INSERT, and SELECT with WHERE equality.
5. **Executor:** Iterator model, sequential scan, index scan, and filter.
6. **Transactions:** BEGIN, COMMIT, ROLLBACK, and before-and-after value logging.
7. **Recovery:** Write-ahead logging, and redo and undo on startup.