SRC=src
PROTO=proto
UTIL=util

all:
	$(MAKE) -C $(PROTO)
	$(MAKE) -C $(SRC)
	$(MAKE) -C $(UTIL)

clean:
	$(MAKE) -C $(SRC) clean
	$(MAKE) -C $(UTIL) clean
