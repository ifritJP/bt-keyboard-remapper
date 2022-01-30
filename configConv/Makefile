LNSC=lnsc

ifneq ($(wildcard make.conf.mk),)
include make.conf.mk
endif

SRCS=
SRCS += $(shell find . -iname '*.lns')

all:
	@echo $(SRCS) | sed 's/ /\n/g' |	\
		$(LNSC) @- save -langGo --main conv $(LNS_OPT)
	$(LNSC) conv.lns mkmain entry.go
	go build -tags gopherlua -o conv

conv.go: conv.lns
	$(LNSC) $< save -langGo --package httpd
