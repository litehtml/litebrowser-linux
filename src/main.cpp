#include "globals.h"
#include "browser_wnd.h"

int on_cmd(const Glib::RefPtr<Gio::ApplicationCommandLine> &, Glib::RefPtr<Gtk::Application> &app)
{
    app->activate();
    return 0;
}
int main (int argc, char *argv[])
{
	Glib::RefPtr<Gtk::Application> app = Gtk::Application::create(argc, argv, "litehtml.browser", Gio::APPLICATION_HANDLES_COMMAND_LINE);

    app->signal_command_line().connect(
            sigc::bind(sigc::ptr_fun(on_cmd), app), false);

	std::string url;
	if(argc > 1)
	{
		url = argv[1];
	}
	browser_window win(url);
	return app->run(win);
}
