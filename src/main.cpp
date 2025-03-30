#include "browser_wnd.h"
#ifdef FOR_TESTING
#include "fonts.h"
#endif

#ifdef LIBADWAITA_AVAILABLE
#include <adwaita.h>
#endif

int main (int argc, char *argv[])
{

#ifdef LIBADWAITA_AVAILABLE
	adw_init();
#endif

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
