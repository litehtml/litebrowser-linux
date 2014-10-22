CC=gcc
CXX=g++
RM=rm -f

LITEHTMLSRC = litehtml/src
CONTAINIERSRC = litehtml/containers/linux

CPPFLAGS=-g -O2 -Wall $(shell pkg-config gdkmm-3.0 gtkmm-3.0 liburiparser libcurl --cflags)
LDFLAGS=-g
LDLIBS=$(shell pkg-config gdkmm-3.0 gtkmm-3.0 liburiparser libcurl --libs) -lfontconfig

SRCS = \
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
$(LITEHTMLSRC)/xh_scanner.cpp \
$(LITEHTMLSRC)/instream.cpp \
$(CONTAINIERSRC)/container_linux.cpp

OBJS = $(subst .cpp,.o,$(SRCS))
OBJS += master.css.o

all: litebrowser

litebrowser: $(OBJS)
	g++ $(LDFLAGS) -o litebrowser $(OBJS) $(LDLIBS) 

depend: .depend

.depend: $(SRCS)
	rm -f ./.depend
	$(CXX) $(CPPFLAGS) -MM $^>>./.depend;

clean:
	$(RM) $(OBJS)

dist-clean: clean
	$(RM) *~ .dependtool

include .depend

master.css.o: litehtml/include/master.css
	ld -r -b binary -o master.css.o litehtml/include/master.css;