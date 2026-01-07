#include "browser_wnd.h"
#include <gdk/gdkkeysyms.h>
#include <sstream>
#include <gtkmm.h>
#include "html_dumper.h"

struct
{
	const char* name;
	const char* url;
} g_bookmarks[] = {
	{"Alexei Navalny (Wiki)", "https://en.wikipedia.org/wiki/Alexei_Navalny?useskin=vector"},
	{"litehtml Web Site", "http://www.litehtml.com/"},
	{"True Launch Bar", "http://www.truelaunchbar.com/"},
	{"Tordex", "http://www.tordex.com/"},
	{"Obama (Wiki)", "https://en.wikipedia.org/wiki/Barack_Obama?useskin=vector"},
	{"Elizabeth II (Wiki)", "https://en.wikipedia.org/wiki/Elizabeth_II?useskin=vector"},
	{"std::vector", "https://en.cppreference.com/w/cpp/container/vector"},
	{"BinaryTides", "https://www.binarytides.com/"},
	{"Ubuntu Forums", "https://ubuntuforums.org/"},
};

static inline void mk_button(Gtk::Button& btn, const std::string& label_text, const std::string& icon_name)
{
	btn.set_focusable(false);
	btn.set_image_from_icon_name(icon_name);
	btn.set_tooltip_text(label_text);
}

browser_window::browser_window(Gio::Application* app, const std::string& url)
{
	auto main_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::VERTICAL);

	set_child(*main_box);
	set_titlebar(m_header);

	m_header.show();

	m_header.set_show_title_buttons(false);

	auto title_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL);
	auto left_box  = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 5);
	auto right_box = Gtk::make_managed<Gtk::Box>(Gtk::Orientation::HORIZONTAL, 5);

	left_box->append(m_back_button);
	mk_button(m_back_button, "Go Back", "go-previous-symbolic");
	m_back_button.signal_clicked().connect(sigc::mem_fun(*this, &browser_window::on_back_clicked));

	left_box->append(m_forward_button);
	mk_button(m_forward_button, "Go Forward", "go-next-symbolic");
	m_forward_button.set_margin_end(10);
	m_forward_button.signal_clicked().connect(sigc::mem_fun(*this, &browser_window::on_forward_clicked));

	left_box->append(m_stop_reload_button);
	mk_button(m_stop_reload_button, "Reload Page", "view-refresh-symbolic");
	m_stop_reload_button.signal_clicked().connect(sigc::mem_fun(*this, &browser_window::on_stop_reload_clicked));

	left_box->append(m_home_button);
	mk_button(m_home_button, "Go Home", "go-home-symbolic");
	m_home_button.signal_clicked().connect(sigc::mem_fun(*this, &browser_window::on_home_clicked));

	left_box->set_hexpand(false);
	title_box->append(*left_box);

	title_box->append(m_address_bar);
	m_address_bar.set_hexpand(true);
	m_address_bar.set_halign(Gtk::Align::FILL);
	m_address_bar.set_margin_start(20);
	m_address_bar.set_margin_end(3);
	m_address_bar.signal_activate().connect(sigc::mem_fun(*this, &browser_window::on_go_clicked));
	m_address_bar.property_primary_icon_name().set_value("insert-link-symbolic");
	m_address_bar.set_text("http://www.litehtml.com/");

	right_box->append(m_go_button);
	mk_button(m_go_button, "Go", "media-playback-start-symbolic");
	m_go_button.set_margin_end(20);
	m_go_button.signal_clicked().connect(sigc::mem_fun(*this, &browser_window::on_go_clicked));

	// Creating bookmarks popover
	auto menu_model = Gio::Menu::create();

	for(const auto& url : g_bookmarks)
	{
		std::string action;
		action += "app.open_url('";
		action += url.url;
		action += "')";
		menu_model->append(url.name, action);
	}

	auto action = Gio::SimpleAction::create("open_url", Glib::VariantType("s"));
	action->signal_activate().connect([this](const Glib::VariantBase& parameter) {
		auto value = parameter.get_dynamic<std::string>();
		m_html.open_url(value);
		m_bookmarks_popover.popdown();
	});
	app->add_action(action);

	m_bookmarks_popover.set_menu_model(menu_model);
	m_bookmarks_popover.set_has_arrow(true);
	m_bookmarks_popover.set_parent(m_bookmarks_button);

	right_box->append(m_bookmarks_button);
	mk_button(m_bookmarks_button, "Bookmarks", "user-bookmarks-symbolic");
	m_bookmarks_button.signal_clicked().connect([this]() { m_bookmarks_popover.popup(); });

	// Creating tools popover
	menu_model			= Gio::Menu::create();
	auto section_render = Gio::Menu::create();
	auto section_draw	= Gio::Menu::create();
	auto section_other	= Gio::Menu::create();

	section_render->append("Single Render", "app.test_render(1)");
	section_render->append("Render 10 Times", "app.test_render(10)");
	section_render->append("Render 100 Times", "app.test_render(100)");
	section_draw->append("Single Draw", "app.test_draw(1)");
	section_draw->append("Draw 10 Times", "app.test_draw(10)");
	section_draw->append("Draw 100 Times", "app.test_draw(100)");
	section_other->append("Append Children from String", "app.append_children(0)");
	section_other->append("Replace Children from String", "app.append_children(1)");
	section_other->append("Dump parsed HTML", "app.dump");

	menu_model->append_section(section_render);
	menu_model->append_section(section_draw);
	menu_model->append_section(section_other);

	m_tools_popover.set_menu_model(menu_model);
	m_tools_popover.set_has_arrow(true);
	m_tools_popover.set_parent(m_tools_button);

	action = Gio::SimpleAction::create("test_render", Glib::VariantType("i"));
	action->signal_activate().connect([this](const Glib::VariantBase& parameter) {
		auto value = parameter.get_dynamic<int>();
		on_render_measure(value);
		m_bookmarks_popover.popdown();
	});
	app->add_action(action);

	action = Gio::SimpleAction::create("test_draw", Glib::VariantType("i"));
	action->signal_activate().connect([this](const Glib::VariantBase& parameter) {
		auto value = parameter.get_dynamic<int>();
		on_draw_measure(value);
		m_bookmarks_popover.popdown();
	});
	app->add_action(action);

	action = Gio::SimpleAction::create("dump");
	action->signal_activate().connect([this](const Glib::VariantBase& /* parameter */) {
		on_dump();
		m_bookmarks_popover.popdown();
	});
	app->add_action(action);

	action = Gio::SimpleAction::create("append_children", Glib::VariantType("i"));
	action->signal_activate().connect([this](const Glib::VariantBase& parameter) {
		auto value = parameter.get_dynamic<int>();
		on_test_append_children_from_string(value == 1);
		m_bookmarks_popover.popdown();
	});
	app->add_action(action);

	right_box->append(m_tools_button);
	mk_button(m_tools_button, "Tools", "preferences-system-symbolic");
	m_tools_button.signal_clicked().connect([this]() { m_tools_popover.popup(); });

	auto win_ctls = Gtk::make_managed<Gtk::WindowControls>(Gtk::PackType::END);
	right_box->append(*win_ctls);

	title_box->append(*right_box);

	title_box->set_halign(Gtk::Align::FILL);
	title_box->set_hexpand(true);
	m_header.set_title_widget(*title_box);
	set_titlebar(m_header);

	main_box->append(m_html);
	m_html.set_expand(true);
	m_html.signal_set_address().connect(sigc::mem_fun(*this, &browser_window::set_address));
	m_html.signal_update_state().connect(sigc::mem_fun(*this, &browser_window::update_buttons));

	signal_close_request().connect(sigc::mem_fun(m_html, &html_widget::on_close), false);

	set_default_size(1280, 720);

	if(!url.empty()) { m_html.open_url(url); }

	update_buttons(0);
}

browser_window::~browser_window()
{
	m_bookmarks_popover.unparent();
	m_tools_popover.unparent();
}

void browser_window::on_go_clicked()
{
	litehtml::string url = m_address_bar.get_text();
	m_html.open_url(url);
	m_html.grab_focus();
}

void browser_window::on_forward_clicked()
{
	m_html.go_forward();
}

void browser_window::on_back_clicked()
{
	m_html.go_back();
}

void browser_window::update_buttons(uint32_t)
{
	uint32_t state = m_html.get_state();

	if((m_prev_state & page_state_has_back) != (state & page_state_has_back))
	{
		m_back_button.set_sensitive(state & page_state_has_back);
	}
	if((m_prev_state & page_state_has_forward) != (state & page_state_has_forward))
	{
		m_forward_button.set_sensitive(state & page_state_has_forward);
	}
	if((m_prev_state & page_state_downloading) != (state & page_state_downloading))
	{
		if(state & page_state_downloading)
		{
			m_stop_reload_button.set_image_from_icon_name("process-stop-symbolic");
		} else
		{
			m_stop_reload_button.set_image_from_icon_name("view-refresh-symbolic");
		}
	}
	m_prev_state = state;
}

void browser_window::on_render_measure(int number)
{
	long time = 0;
	m_html.run_with_document([number, this, &time](const std::shared_ptr<litehtml::document>& doc) {
		auto t1 = std::chrono::high_resolution_clock::now();
		for (int i = 0; i < number; i++)
		{
			doc->render(m_html.get_render_width());
		}
		auto t2 = std::chrono::high_resolution_clock::now();
		time = (std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1)).count();
	});

	std::ostringstream message;

	message << time << " ms for " << number << " times rendering";

	auto dialog = Gtk::AlertDialog::create();
	dialog->set_message(message.str());
	dialog->set_buttons({"OK"});
	dialog->set_modal(true);
	dialog->show(*this);
}

void browser_window::on_draw_measure(int number)
{
	long time = 0;
	m_html.run_with_document([number, this, &time](const std::shared_ptr<litehtml::document>& doc) {
		litehtml::position view_port;
		m_html.get_viewport(view_port);

		int width = view_port.width;
		int height = view_port.height;

		int stride = cairo_format_stride_for_width(CAIRO_FORMAT_ARGB32, width);
		auto image = (unsigned char *) g_malloc(stride * height);

		cairo_surface_t *surface = cairo_image_surface_create_for_data(image, CAIRO_FORMAT_ARGB32, width, height,
																	   stride);
		cairo_t *cr = cairo_create(surface);

		litehtml::position pos;
		pos.width = width;
		pos.height = height;
		pos.x = 0;
		pos.y = 0;

		int x = view_port.x;
		int y = view_port.y;

		cairo_rectangle(cr, 0, 0, width, height);
		cairo_set_source_rgb(cr, 1, 1, 1);
		cairo_paint(cr);
		doc->draw((litehtml::uint_ptr) cr, -x, -y, &pos);
		cairo_surface_write_to_png(surface, "/tmp/litebrowser.png");

		auto t1 = std::chrono::high_resolution_clock::now();
		for (int i = 0; i < number; i++)
		{
			doc->draw((litehtml::uint_ptr) cr, -x, -y, &pos);
		}
		auto t2 = std::chrono::high_resolution_clock::now();

		cairo_destroy(cr);
		cairo_surface_destroy(surface);
		g_free(image);

		time = (std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1)).count();
	});

	std::ostringstream message;
	message << time << " ms for " << number << " times measure";

	auto dialog = Gtk::AlertDialog::create();
	dialog->set_message(message.str());
	dialog->set_buttons({"OK"});
	dialog->set_modal(true);
	dialog->show(*this);
}

void browser_window::on_dump()
{
	html_dumper cout("/tmp/litehtml-dump.txt");
	m_html.run_with_document([&cout](const std::shared_ptr<litehtml::document>& doc) {
		doc->dump(cout);
	});
	auto dialog = Gtk::AlertDialog::create();
	dialog->set_message("File is saved");
	dialog->set_detail("The parsed HTML tree was saved into he file: /tmp/litehtml-dump.txt");
	dialog->set_buttons({"Close"});
	dialog->set_modal(true);
	dialog->show(*this);
}

void browser_window::on_stop_reload_clicked()
{
	uint32_t state = m_html.get_state();
	if(state & page_state_downloading)
	{
		m_html.stop_download();
	} else
	{
		m_html.reload();
	}
}

void browser_window::on_home_clicked()
{
	m_html.open_url("http://www.litehtml.com/");
}

void browser_window::on_test_append_children_from_string(bool replace_existing)
{
	m_html.run_with_document([replace_existing](const std::shared_ptr<litehtml::document>& doc) {
		auto el = doc->root()->select_one("body");
		doc->append_children_from_string(*el, "<h1>Testing append_children_from_string</h1><p>This paragraph was added using append_children_from_string method of document class.</p>", replace_existing);
	});
	m_html.render();
}
