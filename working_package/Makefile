# Self-contained build for the mini project (same source as repo root).
CFLAGS = -Wall -Wextra -std=c11
TARGET = trans
SRC = trans.c

.PHONY: all clean demo run

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $<

clean:
	rm -f $(TARGET) credit.dat accounts.txt accounts_sorted.txt

# Fresh database + sample accounts, then exit (see demo_seed.txt).
demo: all
	rm -f credit.dat accounts.txt accounts_sorted.txt
	./$(TARGET) < demo_seed.txt

run: all
	./$(TARGET)
