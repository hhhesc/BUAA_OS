#include <lib.h>

static char *msg = "This is the NEW message of the day!\n";
static char *diff_msg = "This is a different message of the day!\n";

int main() {
	int r;
	int fdnum;
	char buf[512];
	int n;

	if ((r = open("/newmotd", O_RDWR)) < 0) {
		user_panic("open /newmotd: %d", r);
	}
	fdnum = r;
	r=fork();
	if (r==0){
		r = read(fdnum,buf,15);
		debugf("child get %s\n",buf);
	} else {
		r = read(fdnum,buf,15);
		debugf("father get %s\n",buf);
	}
	return 0;
}
