EXTENSION = pg_extension_template
EXTVERSION   = $(shell grep default_version $(EXTENSION).control | sed -e "s/default_version[[:space:]]*=[[:space:]]*'\([^']*\)'/\1/")
TESTS        = $(wildcard test/sql/*.sql)
REGRESS      = $(patsubst test/sql/%.sql,%,$(TESTS))
REGRESS_OPTS = --inputdir=test

PG_CONFIG ?= pg_config

MODULE_big = pg_extension_template
OBJS = pg_extension_template.o

all:

release-zip: all
	git archive --format zip --prefix=${EXTENSION}-${EXTVERSION}/ --output ./${EXTENSION}-${EXTVERSION}.zip HEAD
	unzip ./${EXTENSION}-$(EXTVERSION).zip
	rm ./${EXTENSION}-$(EXTVERSION).zip
	rm ./${EXTENSION}-$(EXTVERSION)/.gitignore
	sed -i -e "s/__VERSION__/$(EXTVERSION)/g"  ./${EXTENSION}-$(EXTVERSION)/META.json
	zip -r ./${EXTENSION}-$(EXTVERSION).zip ./${EXTENSION}-$(EXTVERSION)/
	rm -rf ./${EXTENSION}-$(EXTVERSION)

DATA = $(wildcard *--*.sql)
PGXS := $(shell $(PG_CONFIG) --pgxs)
include $(PGXS)
