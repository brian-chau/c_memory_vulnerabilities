## Double-free vulnerability

This is similar to the use-after-free vulnerability, which is where we free a heap memory block followed by allocating a new heap memory block of the same size, resulting in both the first and second resulting pointers to point to the same heap memory block.

Double-free vulnerability, however, is when we free a heap memory block twice, resulting in the subsequent two`malloc`s of the same size to return pointers that points to the same memory block as each other as well as the original pointer that we had freed.

How does this work?

Whenever a block of heap memory is freed, the GCC compiler creates a pair of entries in the `tcache_perthread_struct` struct that contains two arrays -- `counts` and `entries`.

If I understand it correctly, each index in the `counts` array corresponds to an index in the `entries` array. Each pair of `counts[i]` and `entries[i]` go together, where `counts[i]` indicates the number of items in the singly-linked list that is pointed to by the `entries[i]` index. The `entries[i]` points to the head `tcache_entry` struct of the singly-linked list that it points to.

Each pair of `counts[i]` and `entries[i]` corresponds to a certain size that is `malloc`d then `free`d. For example, if we `malloc` then `free` a 16-byte heap memory block, then the index `i` in `counts[i]` will be `1` (for there is one item that had been freed that was 16 bytes in size) and `entries[i]` points to the `tcache_entry` that corresponds to that `free`d heap memory block.

Then, every subsequent time a heap memory block is `free`d, the application will first look for the "bin" that size of `malloc`d heap memory, followed by searching down the singly-linked list of that bin for the corresponding `tcache_entry` struct. If it finds it (part of it is comparing the `key` field in the `tcache_entry` struct), then the application will complain and not allow you to `free` that heap memory block, as it found the `tcache_entry` struct that indicates that it has already been freed.

However, this is vulnerable, partly because of the "use-after-free" vulnerability.

Here is what might happen:

1. A pointer to a heap memory block of 16-bytes is created, and the pointer is called `a_ptr`.
2. `a_ptr` is `free`d by the programmer, but `a_ptr` is still valid after the free (i.e. `a_ptr` still points to the heap memory block, even though we already `free`d it).
3. The application will (behind the scenes) create a `tcache_entry` struct at that heap memory location that `a_ptr` points at.
4. Somewhere along the line, the programmer forgets that they had already `free`d the heap memory pointed by `a_ptr`, and will start using it, which is known as a "use-after-free" vulnerability.
5. The usage in (4) will overwrite the second of two pointers in the `tcache_entry` struct that `a_ptr` now points at.<br/>
5.1. Note that `tcache_entry` struct has two pointers, where each pointer is 8 bytes in size -- the first being a pointer to the next `tcache_entry` struct in the singly-linked list, and the second pointer being the key that indicates the "owner" `tcache_perthread_struct` that "owns" the `tcache_entry`.<br/>
5.2. The combination of the two pointers in `tcache_entry` struct indicates the `tcache_entry`s identity, which is what the application/compiler will use to determine if a heap memory location has already been `free`d.
6. The programmer, still not knowing that they had already `free`d the heap memory that `a_ptr` pointed to, will `free` the heap memory that `a_ptr` points to a second time! This results in a "double-free vulnerability." This happens because the app/compiler could not find the `tcache_entry` struct corresponding to the `free`d memory because the user had just overwritten it with the UAF vulnerability.
7. The impact of the "double-free vulnerability" is that the next two heap memory addresses (of the same size? I'm not sure yet) that are allocated using `malloc` will point to the same address as each other, and to the original address of `a_ptr`.<br/>
7.1. For example, if we `malloc`d heap memory two times and stored the pointers as `b_ptr` and `c_ptr`, then both `b_ptr` and `c_ptr` will point to the same address as each other, but also to the same address that `a_ptr` pointed to.<br/>
7.2. In other words, we now have three pointers pointing to the same heap memory, not just two as with a "use-after-free" vulnerability.
