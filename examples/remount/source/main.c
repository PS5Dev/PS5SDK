#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/syscall.h>

#include <ps5/payload_main.h>


/**
 *
 **/
struct iovec {
  void         *iov_base;
  unsigned long iov_len;
};


/**
 * Build an iovec structure for nmount().
 **/
static int
build_iovec(struct iovec **iov, int *iovlen, const char *name, const char *v) {
  int i;

  if(*iovlen < 0) {
    return -1;
  }

  i = *iovlen;
  *iov = realloc(*iov, sizeof(**iov) * (i + 2));
  if(*iov == 0) {
    *iovlen = -1;
    return -1;
  }

  (*iov)[i].iov_base = strdup(name);
  (*iov)[i].iov_len = strlen(name) + 1;
  i++;

  (*iov)[i].iov_base = v ? strdup(v) : 0;
  (*iov)[i].iov_len = v ? strlen(v) + 1 : 0;
  i++;

  *iovlen = i;

  return 0;
}


/**
 * Remount /system with write permissions.
 **/
int
payload_main(void) {
  struct iovec* iov = 0;
  int iovlen = 0;

  build_iovec(&iov, &iovlen, "fstype", "exfatfs");
  build_iovec(&iov, &iovlen, "fspath", "/system");
  build_iovec(&iov, &iovlen, "from", "/dev/ssd0.system");
  build_iovec(&iov, &iovlen, "large", "yes");
  build_iovec(&iov, &iovlen, "timezone", "static");
  build_iovec(&iov, &iovlen, "async", 0);
  build_iovec(&iov, &iovlen, "ignoreacl", 0);

  if(syscall(SYS_nmount, iov, iovlen, MNT_UPDATE)) {
    return -1;
  }

  return 0;
}
