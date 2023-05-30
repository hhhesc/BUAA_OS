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
 u_int pcmdn;
 u_int cmdn;
 u_int offset;
 u_int curenv_id;
int w_history(char *buf){
	int fd = open("/.history",O_WRONLY);
	if (fd<0){
		return fd;
	}
	seek(fd,offset);
	int r = write(fd,buf,strlen(buf));
	write(fd,"\n",1);
	offset+=(r+1);
	//debugf("write in %s\n",buf);
	close(fd);
	cmdn++;
	pcmdn=cmdn;
	return 0;
}

int r_history(char* res){
	int i=0;
	int fdnum = open("/.history",O_RDONLY);
	if (fdnum<0){
		return fdnum;
	}
	int n=0;
	char c;
	memset(res,0,strlen(res));
	if(cmdn>1){
		while (n!=pcmdn){
			read(fdnum,&c,1);
			while(c!='\n'){
				read(fdnum,&c,1);
			}
			n++;
		} //读到pcmdn项的头部
		read(fdnum,&c,1);
	} else if (cmdn==0){
		return -1;
	}
	while(c!='\n'&&c){
		res[i]=c;
		read(fdnum,&c,1);
		i++;
	}
	seek(fdnum,0);
	close(fdnum);
	return 0;	
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
			fd = open(t,O_CREAT | O_WRONLY);
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
		case ';':;
			u_int fork_id = fork();
			*bkstage = 0;
			if (fork_id){
				wait(fork_id);
				return parsecmd(argv,rightpipe,bkstage);
			} else {
				return argc;
			}
			break;
		case '&':
			*bkstage = fork();
			if (*bkstage!=0){
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
	if (argc == 0) {
		return;
	}
	if (strcmp(argv[0],"cd")==0 || strcmp(argv[0],"cd.b")==0){
		u_int id = curenv_id;
		char id2str[100];
		int idlen = 0;
		while (id!=0){
			id2str[idlen] = '0'+id%10;
			id/=10;
			idlen++;
		}
		id2str[idlen] = 0;
		argv[argc] = id2str;
		argc++;
	}
	argv[argc] = 0;

	int child = spawn(argv[0], argv);
	if (child<0){
		debugf("Cmd fail.\n");
	}
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
				if (pcmdn>0){
					pcmdn--;
					r_history(buf);
				}
				printf("$ %s",buf);
				end=strlen(buf);
				i=end;
			} else {
				printf("\x1b[%dC",end-i-1);
				printf("\r");
				for (int i=0;i<=end;i++){
					printf(" ");
				}
				printf("\r");
				char tempbuf[1024];
				strcpy(tempbuf,buf);
				if (pcmdn<cmdn){
					pcmdn++;
					r_history(buf);
				}
				printf("$ %s",buf);
				end=strlen(buf);
				i=end;
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
			printf("\x1b[C");
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
	create("/.history",FTYPE_REG);
	curenv_id = syscall_getenvid();

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
			runcmd(buf);
			exit();
		} else {
			wait(r);
		}
	}
	return 0;
}
