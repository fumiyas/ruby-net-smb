RAKE=		rake
GEM=		gem

default: PHONY build

build: PHONY
	$(RAKE) compile

test t: PHONY
	$(RAKE) test

gem: PHONY
	$(RAKE) build

upload: PHONY
	$(RM) pkg/*.gem
	$(MAKE) gem
	$(GEM) push pkg/*.gem

clean: PHONY
	$(RAKE) clean

distclean: PHONY
	$(RAKE) clobber

PHONY:

