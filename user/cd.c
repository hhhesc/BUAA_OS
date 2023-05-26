#include<lib.h>

void cd(char *path){
	syscall_chdir(path);
	printf("change to %s\n",path);
}

void usage(void){
	printf("usage: cd [path]\n");
	exit();
}

int main(int argc, char **argv){
	ARGBEGIN {
		default:
			usage();
	}
	ARGEND

	if (argc==0){
		user_panic("too few args for cd\n");
	} else {
		for (int i=0;i<argc;i++){
			cd(argv[i]);
		}
	}
	return 0;
}
