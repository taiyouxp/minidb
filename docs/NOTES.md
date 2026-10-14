**M1:**
<p>
The offset byte of the register id=1 reg_num=1001 data=abcd is 0x00002010 (8208) <br>
Most difficulty: to set the structure of the page due to the given minidb design constraints. 
</p>

**M2:** 
<p>
    So the syntax, the implementation of some kinda 'OOP' in C and the language structures have been some constraints to submit modules in the correct deadline. Anyway, without being specific, this 'figuring out' was our burden, like, on the first sight, we didn't even paid attention to the correct structure of M1, which later lead to a 'pager', and this 'pager' is called by the 'cache' functions, for example. Thus, trying to accomplish the instructions given in the slides by translating to the strict and low-level likeness of C has been an adventure. To actually answer (and be more specific) the most difficult decision: when and where a specific function (method) of the cache should call eachother, and so its parameters definitions. Also, i was letting the cache size be hard-coded (defined) as 3, but after some writing of the main (m1+m2 for now, and in the current writing of this, not yet commited), i thought this was not really efficient, as we could be testing different quantities of frames (cache capacity) to measure how many misses, hits and writes could differ on quantity (and so ticks).
</p>
**M3:**
<p>
    It's a node representation. A node is an m3_node struct in memory: is_leaf, n_keys, and a keys[] array. An internal node uses children[] pointers (one more child than keys). A leaf uses recs[], each a (page, slot) location in the table, plus a next pointer chaining the leaves for range searches. The tree maps id -> (page, slot) and the table pages hold the records. Nodes are not stored in 4096-byte pages yet.<br>
    Now the next step is: mapping to a page, so one node = one 4096-byte page. The page header holds is_leaf and n_keys. After it come the keys (4 bytes each), then either child page numbers (internal) or (page, slot) pairs (leaf). The pointers children[] and next become page numbers, so a child could maybe be reached with cache_fix(child_page_no). That means each tree level costs one page read, and it would be written with the same little-endian put32/get32 helpers written on m1 run.
</p>
