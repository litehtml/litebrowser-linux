/*
 * main.cpp
 *
 *  Created on: Jul 31, 2013
 *      Author: tordex
 */
#include "globals.h"
#include "browser_wnd.h"

extern "C" char _binary_litehtml_include_master_css_start;
extern "C" char _binary_litehtml_include_master_css_end;

int main (int argc, char *argv[])
{
	Glib::RefPtr<Gtk::Application> app = Gtk::Application::create(argc, argv, "litehtml.browser");

	litehtml::tstring css;
	css.append(&_binary_litehtml_include_master_css_start, &_binary_litehtml_include_master_css_end - &_binary_litehtml_include_master_css_start);

	litehtml::context html_context;
	html_context.load_master_stylesheet(css.c_str());

	browser_window win(&html_context);

	return app->run(win);
}