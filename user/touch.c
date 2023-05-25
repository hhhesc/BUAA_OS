#include<lib.h>

void touch(char *path){
	int fd ,r;
	if ((fd = open(path,O_RDONLY))>=0){
		user_panic("The file already exisit!\n");
	} else {
		if ((r = create(path,FTYPE_REG))<0){
			user_panic("touch fail.\n");
		}
		printf("touch success!\n");
	}
}

void usage(void){
	printf("usage: touch [path]\n");
	exit();
}

int main(int argc, char** argv){
	ARGBEGIN {
		default:
			usage();
	}
	ARGEND

	if (argc == 0){
		return 0;
	} else {
		for (int i=0;i<argc;i++){
			touch(argv[i]);
		}
	}
	return 0;
}
