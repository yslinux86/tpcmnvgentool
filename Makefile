CC      = gcc
# pkg-config --cflags --libs gtk+-3.0
INCLUDE = -I/usr/include/gtk-3.0 \
          -I/usr/include/glib-2.0 \
          -I/usr/lib/$(shell uname -i)-linux-gnu/glib-2.0/include \
          -I/usr/include/pango-1.0 \
          -I/usr/include/harfbuzz \
          -I/usr/include/cairo \
          -I/usr/include/gdk-pixbuf-2.0 \
          -I/usr/include/atk-1.0
LIB     = -lgtk-3 \
          -lgobject-2.0 \
          -lgio-2.0 \
          -lgdk_pixbuf-2.0 \
          -lglib-2.0
CFLAGS  = $(FLAGS_GLOBAL)
CFLAGS  += -g -fno-strict-aliasing -Werror

OBJECTS = common.o \
          policy_tool.o

TARGET = policy_tool

all: $(TARGET)

$(TARGET):$(OBJECTS)
	$(CC) -o $(TARGET) $(OBJECTS) $(INCLUDE) $(CFLAGS) $(LIB)

.c.o:
	$(CC) $(INCLUDE) $(CFLAGS) -c -o $@ $<

.PHONY:clean
clean:
	rm -rf *.o $(TARGET)
