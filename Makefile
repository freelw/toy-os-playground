# 顶层 Makefile - 代理到 src 目录

.PHONY: all clean

all:
	$(MAKE) -C src all

clean:
	$(MAKE) -C src clean

