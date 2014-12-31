CC=gcc -std=c99
CXX=g++ -std=c++11
RM=rm -f

LITEHTMLSRC = litehtml/src
CONTAINIERSRC = litehtml/containers/linux

CPPFLAGS=-g -O2 -Wall $(shell pkg-config gdkmm-3.0 gtkmm-3.0 liburiparser libcurl --cflags)
LDFLAGS=-g
LDLIBS=$(shell pkg-config gdkmm-3.0 gtkmm-3.0 liburiparser libcurl --libs) -lfontconfig

LITEHTML_SOURCES = \
src/broser_wnd.cpp \
src/html_widget.cpp \
src/main.cpp \
src/utils.cpp \
$(LITEHTMLSRC)/background.cpp \
$(LITEHTMLSRC)/el_body.cpp \
$(LITEHTMLSRC)/el_script.cpp \
$(LITEHTMLSRC)/html_tag.cpp \
$(LITEHTMLSRC)/box.cpp \
$(LITEHTMLSRC)/el_break.cpp \
$(LITEHTMLSRC)/el_space.cpp \
$(LITEHTMLSRC)/iterators.cpp \
$(LITEHTMLSRC)/context.cpp \
$(LITEHTMLSRC)/el_comment.cpp \
$(LITEHTMLSRC)/el_style.cpp \
$(LITEHTMLSRC)/media_query.cpp \
$(LITEHTMLSRC)/css_length.cpp \
$(LITEHTMLSRC)/el_div.cpp \
$(LITEHTMLSRC)/el_table.cpp \
$(LITEHTMLSRC)/style.cpp \
$(LITEHTMLSRC)/css_selector.cpp \
$(LITEHTMLSRC)/element.cpp \
$(LITEHTMLSRC)/el_td.cpp \
$(LITEHTMLSRC)/stylesheet.cpp \
$(LITEHTMLSRC)/document.cpp \
$(LITEHTMLSRC)/el_font.cpp \
$(LITEHTMLSRC)/el_text.cpp \
$(LITEHTMLSRC)/table.cpp \
$(LITEHTMLSRC)/el_anchor.cpp \
$(LITEHTMLSRC)/el_image.cpp \
$(LITEHTMLSRC)/el_title.cpp \
$(LITEHTMLSRC)/el_base.cpp \
$(LITEHTMLSRC)/el_link.cpp \
$(LITEHTMLSRC)/el_tr.cpp \
$(LITEHTMLSRC)/el_cdata.cpp \
$(LITEHTMLSRC)/web_color.cpp \
$(LITEHTMLSRC)/el_before_after.cpp \
$(LITEHTMLSRC)/el_para.cpp \
$(LITEHTMLSRC)/html.cpp \
$(LITEHTMLSRC)/utf8_strings.cpp \
$(CONTAINIERSRC)/container_linux.cpp


GUMBO_SOURCES = \
$(LITEHTMLSRC)/gumbo/attribute.c \
$(LITEHTMLSRC)/gumbo/char_ref.c \
$(LITEHTMLSRC)/gumbo/error.c \
$(LITEHTMLSRC)/gumbo/parser.c \
$(LITEHTMLSRC)/gumbo/string_buffer.c \
$(LITEHTMLSRC)/gumbo/string_piece.c \
$(LITEHTMLSRC)/gumbo/tag.c \
$(LITEHTMLSRC)/gumbo/tokenizer.c \
$(LITEHTMLSRC)/gumbo/utf8.c \
$(LITEHTMLSRC)/gumbo/util.c \
$(LITEHTMLSRC)/gumbo/vector.c \

LITEHTML_OBJS = $(subst .cpp,.o,$(LITEHTML_SOURCES))
GUMBO_OBJS = $(subst .c,.o,$(GUMBO_SOURCES))
LITEHTML_OBJS += master.css.o

all: litebrowser

litebrowser: $(LITEHTML_OBJS) $(GUMBO_OBJS)
	g++ $(LDFLAGS) -o litebrowser $(LITEHTML_OBJS) $(GUMBO_OBJS) $(LDLIBS) 

depend: .depend

.depend: $(LITEHTML_SOURCES) $(GUMBO_SOURCES)
	rm -f ./.depend
	$(CXX) $(CPPFLAGS) -MM $(LITEHTML_SOURCES)>>./.depend;
	$(CC) $(CPPFLAGS) -MM $(GUMBO_SOURCES)>>./.depend;

clean:
	$(RM) $(LITEHTML_OBJS) $(GUMBO_OBJS)

dist-clean: clean
	$(RM) *~ .dependtool

include .depend

master.css.o: litehtml/include/master.css
	ld -r -b binary -o master.css.o litehtml/include/master.css;
