SRC=src
UTIL=util

all:
	$(MAKE) -C $(SRC)
	$(MAKE) -C $(UTIL)

clean:
	$(MAKE) -C $(SRC) clean
	$(MAKE) -C $(UTIL) clean
