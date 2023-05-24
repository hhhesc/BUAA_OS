#include <lib.h>
void treedir(char *path,u_int depth);
void tree (char* path,int num) {
	int r;
	struct Stat st;
	if ((r = stat(path,&st))<0){
		user_panic("stat %s: %d",path,r);
	}
	treedir(path,0);
}

void treedir(char *path,u_int depth){
	int fd, n;
	struct File f;

	if ((fd=open(path,O_RDONLY))<0){
		user_panic("path ""%s"" error.",path);
	}
	while((n = readn(fd,&f,sizeof f)) == sizeof f){
		
		if (f.f_name[0]){
			if (f.f_type == FTYPE_DIR){
				char nextpath[1024];
				strcpy(nextpath,path);
				strcpy(nextpath+strlen(nextpath),"/");
				strcpy(nextpath+strlen(nextpath),f.f_name);
				for (int i=0;i<depth;i++){
					printf("   ");
				}
				printf("|");
				printf("--");
				printf("%s\n",f.f_name);
			//	printf("this is a debug:nextpath is %s\n",nextpath);
				treedir(nextpath,depth+1);
			} else {
				for (int i=0;i<depth;i++){
					printf("   ");
				}
				printf("|");
				printf("--");
				printf("%s\n",f.f_name);
			}
		}
	}	
}

void usage(void){
	printf("usage: ls[path]\n");
	exit();
}

int main(int argc, char** argv){
	int i;

	ARGBEGIN {
		default:
			usage();
	}
	ARGEND

	if (argc==0){
		tree("/",0);
	} else {
		for (int i=0;i<argc;i++){
			tree(argv[i],0);
		}
	}
	printf("\n");
	return 0;
}
