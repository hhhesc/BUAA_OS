#include<lib.h>

void pwd(char *id){
	char cwd[1024];
	syscall_getcwd(cwd);
	printf("%s\n",cwd);
}

void usage(void){
	printf("usage: pwd\n");
	exit();
}

int main(int argc, char **argv){
	ARGBEGIN {
		default:
			usage();
	}
	ARGEND

	pwd(argv[0]);
	return 0;
}
