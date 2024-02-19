#include "globals.h"
#include "browser_wnd.h"
#include <gdk/gdkkeysyms.h>
#include <sstream>

struct
{
    const char* name;
    const char* url;
} g_bookmarks[] =
        {
				{"Alexei Navalny (Wiki)", "https://en.wikipedia.org/wiki/Alexei_Navalny?useskin=vector"},
                {"litehtml Web Site", "http://www.litehtml.com/"},
                {"True Launch Bar", "http://www.truelaunchbar.com/"},
                {"Tordex", "http://www.tordex.com/"},
                {"Obama (Wiki)", "https://en.wikipedia.org/wiki/Barack_Obama?useskin=vector"},
                {"Elizabeth II (Wiki)", "https://en.wikipedia.org/wiki/Elizabeth_II?useskin=vector"},
				{"std::vector", "https://en.cppreference.com/w/cpp/container/vector"},
        };

browser_window::browser_window(const std::string& url) :
		m_prev_state(0),
        m_html(this),

        m_tools_render1("Single Render"),
        m_tools_render10("Render 10 Times"),
        m_tools_render100("Render 100 Times"),

        m_tools_draw1("Single Draw"),
        m_tools_draw10("Draw 10 Times"),
        m_tools_draw100("Draw 100 Times"),

        m_tools_dump("Dump parsed HTML")
{
	add(m_vbox);
	m_vbox.show();

	set_titlebar(m_header);

	m_header.show();

	m_header.set_show_close_button(true);
	m_header.property_spacing().set_value(0);

	m_header.pack_start(m_back_button);
    m_back_button.show();
    m_back_button.signal_clicked().connect( sigc::mem_fun(*this, &browser_window::on_back_clicked) );
    m_back_button.set_image_from_icon_name("go-previous-symbolic", Gtk::ICON_SIZE_BUTTON);

	m_header.pack_start(m_forward_button);
    m_forward_button.show();
    m_forward_button.signal_clicked().connect( sigc::mem_fun(*this, &browser_window::on_forward_clicked) );
    m_forward_button.set_image_from_icon_name("go-next-symbolic", Gtk::ICON_SIZE_BUTTON);

	m_header.pack_start(m_stop_reload_button);
	m_stop_reload_button.show();
	m_stop_reload_button.signal_clicked().connect( sigc::mem_fun(*this, &browser_window::on_stop_reload_clicked) );
	m_stop_reload_button.set_image_from_icon_name("view-refresh-symbolic", Gtk::ICON_SIZE_BUTTON);

	m_header.pack_start(m_home_button);
	m_home_button.show();
	m_home_button.signal_clicked().connect( sigc::mem_fun(*this, &browser_window::on_home_clicked) );
	m_home_button.set_image_from_icon_name("go-home-symbolic", Gtk::ICON_SIZE_BUTTON);

	m_header.set_custom_title(m_address_bar);
	m_address_bar.set_hexpand_set(true);
	m_address_bar.set_hexpand();
	m_address_bar.property_primary_icon_name().set_value("document-open-symbolic");
	m_address_bar.set_margin_start(32);

	m_address_bar.show();
	m_address_bar.set_text("http://www.litehtml.com/");

	m_address_bar.add_events(Gdk::KEY_PRESS_MASK);
	m_address_bar.signal_key_press_event().connect( sigc::mem_fun(*this, &browser_window::on_address_key_press), false );

	m_menu_bookmarks.set_halign(Gtk::ALIGN_END);

	for(const auto& url : g_bookmarks)
	{
		m_menu_items.emplace_back(url.name);
		m_menu_bookmarks.append(m_menu_items.back());
		m_menu_items.back().signal_activate().connect(
				sigc::bind(
						sigc::mem_fun(m_html, &html_widget::open_url),
						litehtml::string(url.url)));
	}
	m_menu_bookmarks.show_all();

	m_header.pack_end(m_tools_button);
	m_tools_button.set_popup(m_menu_tools);
	m_tools_button.show();
	m_tools_button.set_image_from_icon_name("preferences-system-symbolic", Gtk::ICON_SIZE_BUTTON);

	m_header.pack_end(m_bookmarks_button);
    m_bookmarks_button.set_popup(m_menu_bookmarks);
    m_bookmarks_button.show();
    m_bookmarks_button.set_image_from_icon_name("user-bookmarks-symbolic", Gtk::ICON_SIZE_BUTTON);

	m_go_button.signal_clicked().connect( sigc::mem_fun(*this, &browser_window::on_go_clicked) );
	m_go_button.set_margin_end(32);

	m_header.pack_end(m_go_button);
	m_go_button.show();
	m_go_button.set_image_from_icon_name("media-playback-start-symbolic", Gtk::ICON_SIZE_BUTTON);

    m_menu_tools.set_halign(Gtk::ALIGN_END);
    m_menu_tools.append(m_tools_render1);
    m_menu_tools.append(m_tools_render10);
    m_menu_tools.append(m_tools_render100);
    m_menu_tools.append(m_tools_draw1);
    m_menu_tools.append(m_tools_draw10);
    m_menu_tools.append(m_tools_draw100);
    m_menu_tools.append(m_tools_dump);

    m_menu_tools.show_all();

    m_tools_render1.signal_activate().connect(
            sigc::bind(
                    sigc::mem_fun(*this, &browser_window::on_render_measure),
                    1));
    m_tools_render10.signal_activate().connect(
            sigc::bind(
                    sigc::mem_fun(*this, &browser_window::on_render_measure),
                    10));
    m_tools_render100.signal_activate().connect(
            sigc::bind(
                    sigc::mem_fun(*this, &browser_window::on_render_measure),
                    100));

    m_tools_draw1.signal_activate().connect(
            sigc::bind(
                    sigc::mem_fun(*this, &browser_window::on_draw_measure),
                    1));
    m_tools_draw10.signal_activate().connect(
            sigc::bind(
                    sigc::mem_fun(*this, &browser_window::on_draw_measure),
                    10));
    m_tools_draw100.signal_activate().connect(
            sigc::bind(
                    sigc::mem_fun(*this, &browser_window::on_draw_measure),
                    100));

    m_tools_dump.signal_activate().connect(
            sigc::mem_fun(*this, &browser_window::on_dump));

    m_vbox.pack_start(m_scrolled_wnd, Gtk::PACK_EXPAND_WIDGET);
	m_scrolled_wnd.show();

	m_scrolled_wnd.add(m_html);
	m_html.show();

	signal_delete_event().connect(sigc::mem_fun(m_html, &html_widget::on_close), false);

    set_default_size(1280, 720);

	if(!url.empty())
	{
		m_html.open_url(url);
	}

    update_buttons();
}

browser_window::~browser_window()
{

}

void browser_window::on_go_clicked()
{
	litehtml::string url = m_address_bar.get_text();
	m_html.open_url(url);
}

bool browser_window::on_address_key_press(GdkEventKey* event)
{
	if(event->keyval == GDK_KEY_Return)
	{
		m_address_bar.select_region(0, -1);
		on_go_clicked();
		return true;
	}

	return false;
}

void browser_window::on_forward_clicked()
{
	m_html.go_forward();
}

void browser_window::on_back_clicked()
{
	m_html.go_back();
}

void browser_window::update_buttons()
{
	uint32_t state = m_html.get_state();

	if((m_prev_state & page_state_has_back) != (state & page_state_has_back))
	{
		if (state & page_state_has_back)
		{
			m_back_button.set_state(Gtk::STATE_NORMAL);
		} else
		{
			m_back_button.set_state(Gtk::STATE_INSENSITIVE);
		}
	}
	if((m_prev_state & page_state_has_forward) != (state & page_state_has_forward))
	{
		if (state & page_state_has_forward)
		{
			m_forward_button.set_state(Gtk::STATE_NORMAL);
		} else
		{
			m_forward_button.set_state(Gtk::STATE_INSENSITIVE);
		}
	}
	if((m_prev_state & page_state_downloading) != (state & page_state_downloading))
	{
		if (state & page_state_downloading)
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
    std::ostringstream message;

    long time = m_html.render_measure(number);

    message << time << " ms for " << number << " times rendering";

    m_pDialog.reset(new Gtk::MessageDialog(*this, message.str(), false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_CLOSE, true));

    m_pDialog->signal_response().connect(
            sigc::hide(sigc::mem_fun(*m_pDialog, &Gtk::Widget::hide)));
    m_pDialog->show();
}

void browser_window::on_draw_measure(int number)
{
    std::ostringstream message;

    long time = m_html.draw_measure(number);

    message << time << " ms for " << number << " times measure";

    m_pDialog.reset(new Gtk::MessageDialog(*this, message.str(), false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_CLOSE, true));

    m_pDialog->signal_response().connect(
            sigc::hide(sigc::mem_fun(*m_pDialog, &Gtk::Widget::hide)));
    m_pDialog->show();
}

void browser_window::on_dump()
{
    m_html.dump("/tmp/litehtml-dump.txt");
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
