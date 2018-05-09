include ../Make.defines

APPS =	gurl \
	exam_bigheap \
	exam_threadpool \
	exam_map \
	exam_url \
	exam_http

SRC_FILES= ${addsuffix .c,${APPS}}
BIN_FILES= ${addprefix ${BIN_DIR}/,${APPS}}

build:.depend ${BIN_FILES}

.depend:${SRC_FILES}
	gcc -MM -I${INC_DIR} $^ > $@

sinclude .depend

${BIN_DIR}/%:%.o
	${CC} ${CFLAGS} -o $@ $< ${LIBS} 

%.o:%.c
	${CC} ${CFLAGS} -c $< -I${INC_DIR}  -o $@
	
rebuild:clean build

clean:
	rm -f ${CLEAN_FILES}
	cd ${BIN_DIR}; rm -f ${APPS}

.PHONY: build rebuild clean 

