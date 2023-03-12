#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include <ps5/payload_main.h>


typedef struct notify_request {
  char useless1[45];
  char message[3075];
} notify_request_t;


int sceKernelSendNotificationRequest(int, notify_request_t*, size_t, int);


static void notify(const char *s) {
  notify_request_t req;

  bzero(&req, sizeof req);
  strncpy(req.message, s, sizeof req.message);

  sceKernelSendNotificationRequest(0, &req, sizeof req, 0);
}


static int
main(void *args) {
  char buf[255];
  struct payload_args *payload_args = args;

  void* (*f_malloc)(size_t size) = 0;
  int ret = payload_args->dlsym(0x2, "malloc", &f_malloc);
  
  sleep(5);
  
  sprintf(buf, "%x\n", ret);
  notify(buf);

  while(1) {
    sleep(1);
  }
  
  return ret;
}


int payload_main(struct payload_args *args) {
  void* (*f_malloc)(size_t size) = 0;
  int ret = args->dlsym(0x2, "malloc", &f_malloc);
  if(ret || !f_malloc) {
    return ret;
  }
  
  sleep(5);
  notify("Forking...\n");
  if (rfork_thread(RFPROC | RFFDG, f_malloc(0x4000), main, args) < 0) {
    return EXIT_FAILURE;
  } else {
    return EXIT_SUCCESS;
  }
}
