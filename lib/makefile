include ../Make.defines

CFLAGS += 

FILES =	\
	gpub \
	gsub \
	bigheap \
	threadpool \
	url \
	sock \
	map \
	http

LIB_OBJS = ${addsuffix .o,${FILES}}	
LIB_SRCS= ${addsuffix .c,${FILES}}	

build:.depend ${LIB_GPUB}

.depend:${LIB_SRCS}
	gcc -MM -I${INC_DIR} $^ > $@   

sinclude .depend                      

${LIB_GPUB}:${LIB_OBJS}
	ar rv ${LIB_GPUB} ${LIB_OBJS} 

%.o:%.c
	${CC} ${CFLAGS} -I${INC_DIR} -c $< -o $@

rebuild:clean build

clean:
	rm -f ${CLEAN_FILES}
	rm -f ${LIB_GPUB}

.PHONY: build rebuild clean
