#include<lib.h>

void history(){
	long n;
	int fd = open("/.history",O_RDONLY);
	if (fd<0){
		user_panic("history open fail\n");
	}
	char buf[1024];
	
	while ((n=read(fd,buf,(long)sizeof buf))>0){
		printf("%s",buf);
	}
}

void usage(){
	printf("usage: histroy\n");
	exit();
}

int main(int argc, char** argv){
	ARGBEGIN {
		default:
			usage();
	}
	ARGEND

	
	history();
	return 0;
}
