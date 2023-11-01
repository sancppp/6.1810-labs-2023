# FS

xv6手册的第八章，介绍了xv6的文件系统
- 8.1：总览，抽象出了哪几层
- 8.2-8.3：bcache。bio.c
- 8.4-8.6:logging, log.c
- 8.7:allocator, fs.c
- 8.8-8.10:inode, fs.c
- 8.11-8.14:各种抽象层：directory;path;fd,syscall
- 8.15:linux

## Large files
当前xv6的文件块数限制是12+256个块，一个block 1024 bytes。
inode层：
要分配一个新的 inode（例如在创建文件时），xv6 会调用 ialloc（kernel/fs.c:199）。ialloc 与 balloc 类似：它在磁盘上的 inode 结构中循环，每次一个块，寻找标记为空闲的一个。找到后，它会将新类型写入磁盘，然后通过尾部调用 iget 从 inode 表返回一个条目（kernel/fs.c:213）。
iget (kernel/fs.c:247) 在 inode 表中查找具有所需设备和 inode 编号的活动条目（ip->ref > 0）。如果找到，它就会返回对该 inode 的新引用（kernel/fs.c:256-260）。在 iget 扫描过程中，它会记录第一个空槽的位置（kernel/fs.c:261-262），如果需要分配表项，就会使用该位置。
在读取或写入其元数据或内容之前，代码必须使用 ilock 锁定 inode。一旦 ilock 拥有对 inode 的独占访问权限，它就会根据需要从磁盘（更可能是从缓冲缓存）读取 inode。函数 iunlock（kernel/fs.c:321）释放睡眠锁，这可能会导致任何正在睡眠的进程被唤醒。 
iput（kernel/fs.c:337）通过递减引用计数（kernel/fs.c:360）来释放指向 inode 的 C 指针。如果这是最后一次引用，则该 inode 在 inode 表中的槽现在是空闲的，可以重新用于其他 inode。  
如果 iput 发现没有对某个 inode 的 C 指针引用，并且该 inode 没有指向它的链接（不在目录中出现），则必须释放该 inode 及其数据块。 iput 调用 itrunc 将文件截断至零字节，释放数据块；将 inode 类型设置为 0（未分配）；并将 inode 写入磁盘 (kernel/fs.c:342)
---
sys_open系统调用中，如果是创建一个新的文件，则会调用create创建一个inode（本质是ialloc）

任务：
12 + 256 -> 11 + 256 + 256*256
双向引用块


## Symbolic links

加一个软连接功能，
hint:
执行 symlink(target, path) 系统调用，在 path 处创建指向 target 的新符号链接。请注意，系统调用成功时，target 并不需要存在。symlink 应返回一个代表成功（0）或失败（-1）的整数，这与 link 和 unlink 类似。
修改 open 系统调用，以处理路径指向符号链接的情况。如果文件不存在，open 必须失败。当进程在打开标志中指定 O_NOFOLLOW 时，open 应打开符号链接（而不是跟随符号链接）。
如果链接文件也是符号链接，则必须递归跟踪，直到找到非链接文件。如果链接形成循环，则必须返回错误代码。如果链接深度达到某个阈值（如 10），则可以通过返回错误代码来近似处理。
其他系统调用（如 link 和 unlink）不得跟随符号链接；这些系统调用对符号链接本身进行操作。
本实验无需处理指向目录的符号链接。
软连接会新建一个inode，其内容是指向对象的地址