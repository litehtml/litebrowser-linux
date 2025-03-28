#include "browser_wnd.h"
#ifdef FOR_TESTING
#include "fonts.h"
#endif
#include <adwaita.h>

int on_cmd(const Glib::RefPtr<Gio::ApplicationCommandLine> &, Glib::RefPtr<Gtk::Application> &app)
{
    app->activate();
    return 0;
}
int main (int argc, char *argv[])
{
	adw_init();
#ifdef FOR_TESTING
	prepare_fonts_for_testing();
#endif
	std::string url;
	if(argc > 1)
	{
		url = argv[1];
	}
	// Open the main window
	auto app = Gtk::Application::create("litehtml.testsuite");
	return app->make_window_and_run<browser_window>(argc, argv, app, url);
}
