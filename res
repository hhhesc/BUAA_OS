In file included from ../include/pmap.h:6,
                 from elfloader.c:2:
../include/queue.h:118:8: error: expected ‘=’, ‘,’, ‘;’, ‘asm’ or ‘__attribute__’ before ‘->’ token
  118 |   (elm)->field.le_after = (listelm)->field.le_after;
      |        ^~
../include/queue.h:119:13: error: expected declaration specifiers or ‘...’ before ‘(’ token
  119 |   LIST_NEXT((elm),field) = (listelm);
      |             ^
../include/queue.h:119:19: error: unknown type name ‘field’
  119 |   LIST_NEXT((elm),field) = (listelm);
      |                   ^~~~~
../include/queue.h:120:13: error: expected ‘=’, ‘,’, ‘;’, ‘asm’ or ‘__attribute__’ before ‘->’ token
  120 |   *(listelm)->field.le_after = elm;
      |             ^~
../include/queue.h:121:12: error: expected ‘=’, ‘,’, ‘;’, ‘asm’ or ‘__attribute__’ before ‘->’ token
  121 |   (listelm)->field.le_after = &LIST_NEXT((elm),field);
      |            ^~
../include/queue.h:122:2: error: expected identifier or ‘(’ before ‘}’ token
  122 |  }while(0)
      |  ^
../include/queue.h:122:3: error: expected identifier or ‘(’ before ‘while’
  122 |  }while(0)
      |   ^~~~~
make[2]: *** [Makefile:6：elfloader.o] 错误 1
make[1]: *** [Makefile:57：lib] 错误 2
make: *** [Makefile:40：clean-and-all] 错误 2
