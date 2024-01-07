#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int linuxOpen(const char *path, int oflag)
{
   return open(path, oflag);
}

int linuxClose(int fd)
{
   return close(fd);
}

ssize_t linuxRead(int fd, void *buf, size_t nbytes)
{
   return read(fd, buf, nbytes);
}

ssize_t linuxWrite(int fd, const void *buf, size_t nbytes)
{
   return write(fd, buf, nbytes);
}
