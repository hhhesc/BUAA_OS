#include <args.h>
#include <lib.h>

#define WHITESPACE " \t\r\n"
#define SYMBOLS "<|>&;()"

/* Overview:
 *   Parse the next token from the string at s.
 *
 * Post-Condition:
 *   Set '*p1' to the beginning of the token and '*p2' to just past the token.
 *   Return:
 *     - 0 if the end of string is reached.
 *     - '<' for < (stdin redirection).
 *     - '>' for > (stdout redirection).
 *     - '|' for | (pipe).
 *     - 'w' for a word (command, argument, or file name).
 *
 *   The buffer is modified to turn the spaces after words into zero bytes ('\0'), so that the
 *   returned token is a null-terminated string.
 */
 char* pnextcmd;
int w_history(char *buf){
	int fd = open(".history",O_WRONLY);
	if (fd<0){
		return fd;
	}
	write(fd,buf,strlen(buf));
	write(fd,"\n",1);
	//debugf("write in %s\n",buf);
	((struct Fd*)num2fd(fd))->fd_offset=0;
	close(fd);
	pnextcmd=0;
	return 0;
}

int r_history(char* res,u_int back){
	int i=0;
	char *beginva;
	char *endva;
	int fdnum = open(".history",O_RDONLY);
	struct Fd *fd =(struct Fd*) num2fd(fdnum);
	if (pnextcmd==0){
		beginva = fd2data(fd);
		endva = beginva+((struct Filefd*)fd)->f_file.f_size;
		debugf("beginva = %x,endva = %x\n",beginva,endva);
		pnextcmd = endva;
	}
	close(fdnum);
	if (endva==beginva){
		return -1;
	}
	if (back) {
		debugf("back here,p = %x\n",pnextcmd);
		for (u_int va=beginva;va<=endva;va++){
			printf("%c",*(char*)va);
		}
		printf("\n");
		while(*pnextcmd==0 || *pnextcmd=='\n'){
			pnextcmd--; 
		}
		while(*pnextcmd!='\n' && pnextcmd>beginva) pnextcmd--;
		debugf("p = %x\n",pnextcmd);
		if (*pnextcmd=='\n'){
			pnextcmd++;
		}
		char *p=pnextcmd;
		while(*p && *p!='\n'){
			res[i]=*p;
			p++;
			i++;
		}
		res[i]=0;
		return 0;
	} else {
		while (*pnextcmd=='\n'){
			pnextcmd++;
		}
		while(*pnextcmd && *pnextcmd!='\n'){
			res[i]=*pnextcmd;
			pnextcmd++;
			i++;
		}
		res[i]=0;
		return 0;
	}
}

int _gettoken(char *s, char **p1, char **p2) {
	*p1 = 0;
	*p2 = 0;
	if (s == 0) {
		return 0;
	}

	while (strchr(WHITESPACE, *s)) {
		*s++ = 0;
	}
	if (*s == 0) {
		return 0;
	}

	if (strchr(SYMBOLS, *s)) {
		int t = *s;
		*p1 = s;
		*s++ = 0;
		*p2 = s;
		return t;
	}
	if (*s == '\"') {
		s++;
		*p1 = s;
		while (*s && *s != '\"' && *(s-1) != '\\') {
			s++;
		}
		*s++=0;
		*p2 = s;
		return 'w';
	}

	*p1 = s;
	while (*s && !strchr(WHITESPACE SYMBOLS, *s)) {
		s++;
	}
	*p2 = s;
	return 'w';
}

int gettoken(char *s, char **p1) {
	static int c, nc;
	static char *np1, *np2;

	if (s) {
		nc = _gettoken(s, &np1, &np2);
		return 0;
	}
	c = nc;
	*p1 = np1;
	nc = _gettoken(np2, &np1, &np2);
	return c;
}

#define MAXARGS 128

int parsecmd(char **argv, int *rightpipe, int *bkstage) {
	int argc = 0;
	while (1) {
		char *t;
		int fd;
		int c = gettoken(0, &t);
		switch (c) {
		case 0:
			return argc;
		case 'w':
			if (argc >= MAXARGS) {
				debugf("too many arguments\n");
				exit();
			}
			argv[argc++] = t;
			break;
		case '<':
			if (gettoken(0, &t) != 'w') {
				debugf("syntax error: < not followed by word\n");
				exit();
			}
			fd = open(t,O_RDONLY);
			dup(fd,0);
			close(fd);
			break;
		case '>':
			if (gettoken(0, &t) != 'w') {
				debugf("syntax error: > not followed by word\n");
				exit();
			}
			fd = open(t,O_WRONLY);
			if (fd<0){
				create(t,FTYPE_DIR);
			}
			fd = open(t,O_WRONLY);
			dup(fd,1);
			close(fd);
			break;
		case '|':;
			int p[2];
			pipe(p);
			*rightpipe = fork();
			if (*rightpipe==0){
				dup(p[0],0);
				close(p[0]);
				close(p[1]);
				return parsecmd(argv,rightpipe,bkstage);
			} else {
				dup(p[1],1);
				close(p[1]);
				close(p[0]);
				return argc;
			}
			break;
		case ';':
			*rightpipe = fork();
			if (*rightpipe==0){
				return parsecmd(argv,rightpipe,bkstage);
			} else {
				return argc;
			}
			break;
		case '&':
			*bkstage = fork();
			if (*bkstage==0){
				return parsecmd(argv,rightpipe,bkstage);
			} else {
				return argc;
			}
			break;
		}
	}
	return argc;
}

void runcmd(char *s) {
	gettoken(s, 0);

	char *argv[MAXARGS];
	int rightpipe = 0;
	int bkstage = 0;
	int argc = parsecmd(argv, &rightpipe, &bkstage);
//	debugf("get here\n");
	if (argc == 0) {
		return;
	}
	argv[argc] = 0;

//	debugf("argv[0]=%s,argv[1]=%s\n",argv[0],argv[1]);
	int child = spawn(argv[0], argv);
	close_all();
	if (child >= 0 && !bkstage) {
		wait(child);
	}
	if (rightpipe) {
		wait(rightpipe);
	}
	exit();
}

void readline(char *buf, u_int n) {
	int r;
	char temp;
	int end = 0;
	for (int i = 0; i < n; i++,end++) {
		if ((r = read(0, &temp, 1)) != 1) {
			if (r < 0) {
				debugf("read error: %d\n", r);
			}
			exit();
		}
		if (temp =='\E'){
			read(0,&temp,1);
			read(0,&temp,1);
			if (temp=='D'){
				i-=2;
				end--;
			} else if (temp=='C'){
				end--;
			} else if (temp=='A'){
				printf("\x1b[1B");
				printf("\x1b[%dC",end-i-1);
				printf("\r");
				for (int i=0;i<=end;i++){
					printf(" ");
				}
				printf("\r");
				char tempbuf[1024];
				strcpy(tempbuf,buf);
				int r = r_history(buf,1);
				if (r<0){
					strcpy(buf,tempbuf);
				}
				printf("$ %s",buf);
				end=strlen(buf);
			} else {
				end--;
			}

		} else if (temp == '\b' || temp == 0x7f) {
			//printf("%c",buf[end-1]);
			for (int j=i-1;j<=end;j++){
				buf[j] = buf[j+1];
			}
			printf("\r");
			for (int i=0;i<=end+1;i++){
				printf(" ");
			}
			printf("\r");
			printf("$ %s",buf);
			if (end>i){
				printf("\x1b[%dD",end-i);
			}
			if (temp != '\b') {
				printf("\b");
			}
			if (i > 0) {
				i -= 2;
				end-=2;
			} else {
				i = -1;
				end--;
			}
		} else if (temp == '\r' || temp == '\n') {
			buf[end] = 0;
			return;
		} else {
			printf("\x1b[%dC",end-i); //光标移动到行末
			printf("\r"); 
			for (int i=0;i<=end;i++){
				printf(" ");
			} //删除已经输出的所有内容
			printf("\r");//光标移动到行首
			for (int j=end-1;j>=i;j--){
				buf[j+1]=buf[j];
			}
			buf[i] = temp; //插入输入字符
			buf[end+1]=0; //截取
			printf("$ %s",buf); //重新输出新的字符
			if (end>i){
				printf("\x1b[%dD",end-i); //光标移动到原来的位置
			}
		}
	}
	debugf("line too long\n");
	while ((r = read(0, buf, 1)) == 1 && buf[0] != '\r' && buf[0] != '\n') {
		;
	}
	buf[0] = 0;
}

char buf[1024];

void usage(void) {
	debugf("usage: sh [-dix] [command-file]\n");
	exit();
}


int main(int argc, char **argv) {
	int r;
	int interactive = iscons(0);
//	int interactive = 0;
	int echocmds = 0;
	debugf("\n:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
	debugf("::                                                         ::\n");
	debugf("::                     MOS Shell 2023                      ::\n");
	debugf("::                                                         ::\n");
	debugf(":::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::\n");
	ARGBEGIN {
	case 'i':
		interactive = 1;
		break;
	case 'x':
		echocmds = 1;
		break;
	default:
		usage();
	}
	ARGEND
	create(".history",FTYPE_REG);

	if (argc > 1) {
		usage();
	}
	if (argc == 1) {
		close(0);
		if ((r = open(argv[1], O_RDONLY)) < 0) {
			user_panic("open %s: %d", argv[1], r);
		}
		user_assert(r == 0);
	}
	for (;;) {
		if (interactive) {
			printf("\n$ ");
		}
		readline(buf, sizeof buf);
		w_history(buf);

		if (buf[0] == '#') {
			continue;
		}
		if (echocmds) {
			printf("# %s\n", buf);
		}
		if ((r = fork()) < 0) {
			user_panic("fork: %d", r);
		}
		if (r == 0) {
			debugf("read buf = %s\n",buf);
			runcmd(buf);
			exit();
		} else {
			wait(r);
		}
	}
	return 0;
}
