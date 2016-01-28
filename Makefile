SPECS = spec/*.js 
REPORTER = spec

build: 
	node-gyp distclean configure build

test:
	@NODE_ENV=$(NODE_ENV) ./node_modules/.bin/mocha \
		--require should \
		--reporter $(REPORTER) \
		--ui bdd \
		$(SPECS)

install:
	npm install

.PHONY: build test install
