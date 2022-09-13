#include "globals.h"
#include "browser_wnd.h"

char master_css[] = 
{
#include "master.css.inc"
,0
};

int on_cmd(const Glib::RefPtr<Gio::ApplicationCommandLine> &, Glib::RefPtr<Gtk::Application> &app)
{
    app->activate();
    return 0;
}
int main (int argc, char *argv[])
{
	Glib::RefPtr<Gtk::Application> app = Gtk::Application::create(argc, argv, "litehtml.browser", Gio::APPLICATION_HANDLES_COMMAND_LINE);

	litehtml::context html_context;
	html_context.load_master_stylesheet(master_css);

    app->signal_command_line().connect(
            sigc::bind(sigc::ptr_fun(on_cmd), app), false);

	browser_window win(&html_context);

    if(argc > 1)
    {
        win.open_url(argv[1]);
    }

	return app->run(win);
}
