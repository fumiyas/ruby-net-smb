default: PHONY build

build: PHONY
	rake compile

test t: PHONY
	rake test

gem: PHONY
	rake build

upload: PHONY
	rm -f pkg/*.gem
	$(MAKE) gem
	gem push pkg/*.gem

clean: PHONY
	rake clean

distclean: PHONY
	rake clobber

PHONY:
