.PHONY: all frontend ir-builder optimizations backend clean test-builder test-parser test-semantic test-optimizations test-backend

all: frontend ir-builder optimizations backend

frontend:
	$(MAKE) -C frontend

ir-builder:
	$(MAKE) -C ir_builder

optimizations:
	$(MAKE) -C optimizations

backend:
	$(MAKE) -C backend

test-parser:
	$(MAKE) -C frontend parser-tests

test-semantic:
	$(MAKE) -C frontend semantic-tests

test-builder:
	$(MAKE) -C ir_builder test-builder

test-optimizations:
	$(MAKE) -C optimizations test

test-backend:
	$(MAKE) -C backend test

clean:
	$(MAKE) -C frontend clean
	$(MAKE) -C ir_builder clean
	$(MAKE) -C optimizations clean
	$(MAKE) -C backend clean
