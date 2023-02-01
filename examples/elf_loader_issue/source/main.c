// Required header
#include <ps5/payload_main.h>


static char* strings[] = {
  "ABCD",
  "EFGH",
  "IJKL",
  "MNOP"
};


int payload_main(struct payload_args *args)
{
	return strings[2][2];
}
