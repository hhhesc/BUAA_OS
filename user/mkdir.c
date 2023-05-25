#include<lib.h>

void mkdir(char *path){
	int fd ,r;
	if ((fd = open(path,O_RDONLY))>=0){
		user_panic("The file already exisit!\n");
	} else {
		if ((r = create(path,FTYPE_DIR))<0){
			user_panic("mkdir fail.\n");
		}
		printf("mkdir success!\n");
	}
}

void usage(void){
	printf("usage: mkdir [path]\n");
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
			mkdir(argv[i]);
		}
	}
	return 0;
}
