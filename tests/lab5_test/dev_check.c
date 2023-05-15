#include "lib.h"

int main() {
	
	char test_file[200];
	int fd;
	fd = open("/newmotd",O_RDWR);
	int r = fork();
	if (r==0){
		r = read(fd,test_file,200);
		debugf("child get %s\n",test_file);
	} else {
		r = read(fd,test_file,200);
		debugf("father get %s\n",test_file);
	}

	return 0;
}

