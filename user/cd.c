#include<lib.h>

void cd(char *path,char *parentid){
	if (path[0]!='/') {
		char newpath[1024];
		syscall_getcwd(newpath);
		if (strcmp(path,".")==0){
		} else if (strcmp(path,"..")==0){
			if (strcmp(newpath,"/")!=0){
				for (int j=strlen(newpath)-1;j>=0;j--){
					if (newpath[j]=='/'){
						newpath[j]=0;
						break;
					}
				}
			}
		} else {
			if (strcmp(newpath,"/")!=0){
				strcpy(newpath+strlen(newpath),"/");
			}
			strcpy(newpath+strlen(newpath),path);
		}
		path = newpath;
	}
	int len = strlen(parentid);
	u_int id = 0;
	for (int i=len-1;i>=0;i--){
		id*=10;
		id+=parentid[i]-'0';
	}
	syscall_chdir(path,id);
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

	if (argc<2){
		user_panic("too few args for cd\n");
	} else if (argc==2){
		cd(argv[0],argv[1]);
	} else {
		user_panic("too many args for cd\n");
	}
	return 0;
}
