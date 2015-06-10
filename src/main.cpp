#include "globals.h"
#include "browser_wnd.h"

char master_css[] = 
{
#include "master.css.inc"
,0
};

extern "C" char _binary_litehtml_include_master_css_start;
extern "C" char _binary_litehtml_include_master_css_end;

int main (int argc, char *argv[])
{
	Glib::RefPtr<Gtk::Application> app = Gtk::Application::create(argc, argv, "litehtml.browser");

	litehtml::context html_context;
	html_context.load_master_stylesheet(master_css);

	browser_window win(&html_context);

	return app->run(win);
}
