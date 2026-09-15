CFLAGS = -Wall -Wextra -Werror -pthread
LDLIBS = -lcrypto
TARGET = dnsseed
RM = rm -rf

OBJDIR = build
SRCS = $(wildcard src/*.c)
OBJS = $(SRCS:src/%.c=$(OBJDIR)/%.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LDLIBS)

$(OBJDIR)/%.o: src/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $(TARGET)

$(OBJDIR):
	mkdir -p $(OBJDIR)

clean: 
	$(RM) $(OBJDIR)

fclean: clean
	$(RM) $(TARGET)

re: fclean all

test: all
	bash tests/run.sh
