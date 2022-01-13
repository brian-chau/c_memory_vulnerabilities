## Double-free vulnerability

This is similar to the use-after-free vulnerability, which is when we free a heap memory block that is pointed to by a pointer followed by allocating a new memory block while also re-using the same old pointer, then both pointers will point to the same memory block.

Double-free vulnerability, however, is when we free a heap memory block twice, resulting in the next two `malloc`s to return a pointer that points to the same memory block.

How does this work?

Whenever a block of heap memory is freed, the GCC compiler creates an entry in the `tcache_perthread_struct` structure, which contains two arrays -- `counts` and `entries`.

If I understand it correctly, each index in the `counts` array corresponds to each index in the `entries` array. Each pair of `counts[i]` and `entries[i]` acts as a singly-linked list, where `counts[i]` indicates how many items are in the list, and `entries[i]` points to the head `tcache_entry` struct variable in the singly-linked list.

Each pair of `counts[i]` and `entries[i]` corresponds to a certain size that is `malloc`d. For example, when we `malloc` and then `free` a heap memory block of 16 bytes, then the application will create a pairing of `counts[i]` and `entries[i]` in the `tcache_perthread_cache` struct, where the `entries[i]` points to a `tcache_entry` struct. Each pairing of `counts[i]` and `entries[i]` corresponds to a `malloc`d size. So in this case, the index `i` corresponds to a 16-byte `malloc`d then `free`d heap memory block.

So, every subsequent time a block of heap memory is `free`d, the application will actually look through the singly-linked list for that size of `malloc`d memory looking for the `tcache_entry` struct corresponding to the memory that you already freed. If it finds it (part of it is comparing the `key` field in the `tcache_entry` struct), then the application will complain and not allow you to `free` that heap memory block, as it found the `tcache_entry` struct that indicates that it has already been freed.

However, this is vulnerable, partly because of the "use-after-free" vulnerability.

Here is what might happen:

1. A pointer to a heap memory block of 16-bytes is created, and the pointer is called `a_ptr`.
2. `a_ptr` is `free`d by the programmer, but `a_ptr` is still valid after the free (i.e. `a_ptr` still points to the heap memory block, even though we already `free`d it).
3. The application will (behind the scenes) create a `tcache_entry` struct at that heap memory location that `a_ptr` points at.
4. Somewhere along the line, the programmer forgets that they had already `free`d the heap memory pointed by `a_ptr`, and will start using it (i.e. do a "use-after-free" vulnerability")
5. The usage in (4) will overwrite the second of two pointers in the `tcache_entry` struct that `a_ptr` now points at.<br/>
5.1. Note that `tcache_entry` struct has two pointers (each pointer of size 8 bytes) -- the first being a pointer to the next `tcache_entry` struct in the singly-linked list, and the second pointer being the key that indicates the "owner" `tcache_perthread_struct` that "owns" the `tcache_entry`.<br/>
5.2. The combination of the two pointers in `tcache_entry` struct indicate its identity, which is how the application/compiler will know if a heap memory location has already been `free`d.
6. The programmer, still not knowing that they had already `free`d the heap memory that `a_ptr` pointed to, they will `free` the heap memory that `a_ptr` points to, which now results in a "double-free vulnerability."
7. The impact of the "double-free vulnerability" is that the next two heap memory addresses (of the same size? I'm not sure yet) that are allocated using `malloc` will point to the same address as each other, and to the original address of `a_ptr`.
7.1. For example, if we `malloc`d heap memory two times and stored the pointers as `b_ptr` and `c_ptr`, then both `b_ptr` and `c_ptr` will point to the same address as each other, but also to the same address that `a_ptr` pointed to.
7.2. In other words, we now have three pointers pointing to the same heap memory, not just two as with a "use-after-free" vulnerability.
