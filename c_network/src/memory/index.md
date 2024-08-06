# Memory

* `malloc(size_t size)`: Allocates memory, returns a pointer to the allocated memory.
* `calloc(size_t num, size_t size)`: 1st parmaeter is the size of the elements type, 2nd parameter is the number of elements.
* `realloc(void *ptr, size_t size)`: Resizes the memory block pointed to by `ptr` to `size` bytes. If `ptr` is `NULL`, then the function behaves like `malloc(size)`.

   ```c
   double *ptr;
   ptr = calloc(sizeof(double), 10);
   ptr = realloc(ptr, 20);
   ```

* `memcpy(void *dest, const void *src, size_t n)`: Copies `n` bytes from memory area `src` to memory area `dest`.
* `memset(void *dest, int c, size_t n)`: Set `n` bytes of memory area `dest` to the value `c`.
* `mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)`: Creates a new mapping in the virtual address space of the calling process.
  * `addr`: The starting address for the new mapping. If addr is NULL, then the kernel chooses the address at which to create the mapping; this is the most portable method of creating a new mapping.
  * `length`: The length of the mapping.
  * `prot`: The desired memory protection of the mapping. Common values are: `PROT_READ`, `PROT_WRITE`, `PROT_EXEC`, `PROT_NONE`
  * `flags`: The type of the mapping. Common values are: `MAP_SHARED`, `MAP_PRIVATE`, `MAP_ANONYMOUS`
  * `fd`: The file descriptor of the file which is mapped. If `MAP_ANONYMOUS` is set, `fd` is ignored and should be -1.
  * `offset`: The offset in the file where the mapping starts. Must be a multiple of the page size (usually 4096).
* `munmap(void *addr, size_t length)`: Deletes the mapping for the specified address range.

## Shared Memory

MacOS does not support `shmget` function, but it does support `shm_open` and `mmap` function. So please use `shm_open` and `mmap` to create shared memory on MacOS.
