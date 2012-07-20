default: build

build:
	rake compile

test:
	rake test

gem:
	rake build

upload:
	rm pkg/*.gem
	$(MAKE) gem
	gem push pkg/*.gem

clean:
	rake clean

distclean:
	rake clobber

