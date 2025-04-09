#include "browser_wnd.h"
#ifdef FOR_TESTING
#include "fonts.h"
#endif

#ifdef LIBADWAITA_AVAILABLE
#include <adwaita.h>
#endif

class browser_app : public Gtk::Application
{
  private:
	std::string m_url;

  public:
	browser_app(const std::string& url) :
		Gtk::Application("litehtml.browser", Gio::Application::Flags::HANDLES_COMMAND_LINE),
		m_url(url)
	{
	}

  protected:
	void on_activate() override
	{
		auto window = new browser_window(this, m_url);
		add_window(*window);
		window->present();
	}

	int on_command_line(const Glib::RefPtr<Gio::ApplicationCommandLine>& command_line) override
	{
		int	 argc = 0;
		auto args = command_line->get_arguments(argc);
		if(argc > 1) { m_url = args[1]; }

		activate();

		return 0;
	}
};

int main(int argc, char* argv[])
{
#ifdef LIBADWAITA_AVAILABLE
	adw_init();
#endif

#ifdef FOR_TESTING
	prepare_fonts_for_testing();
#endif

	auto app = Glib::make_refptr_for_instance<browser_app>(new browser_app(""));
	return app->run(argc, argv);
}
