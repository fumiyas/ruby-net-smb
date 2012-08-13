PHONY:

default: PHONY build

build: PHONY
	rake compile

test: PHONY
	rake test

gem: PHONY
	rake build

upload: PHONY
	rm pkg/*.gem
	$(MAKE) gem
	gem push pkg/*.gem

clean: PHONY
	rake clean

distclean: PHONY
	rake clobber

