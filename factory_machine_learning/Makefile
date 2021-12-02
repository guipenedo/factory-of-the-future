SUBDIRS = $(shell ls -d ./factory/*/)
all:
	for dir in $(SUBDIRS) ; do \
		make -C  $$dir ; \
	done
