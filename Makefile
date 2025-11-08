# 顶层 Makefile - 代理到 src 目录

.PHONY: all clean liblansys

all:
	$(MAKE) -C src all

clean:
	$(MAKE) -C src clean

liblansys:
	$(MAKE) -C src/common/lib

