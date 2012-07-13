default: build

build: gem

gem:
	rm pkg/*.gem
	rake build

upload:
	$(MAKE) gem
	gem push pkg/*.gem

clean:
	rake clean

distclean:
	rake clobber

