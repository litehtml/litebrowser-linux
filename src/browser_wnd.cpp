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
                {"litehtml Web Site", "http://www.litehtml.com/"},
                {"True Launch Bar", "http://www.truelaunchbar.com/"},
                {"Tordex", "http://www.tordex.com/"},
                {"Obama (Wiki)", "https://en.wikipedia.org/wiki/Barack_Obama"},
                {"Elizabeth II", "https://en.wikipedia.org/wiki/Elizabeth_II"},
                {"std::vector", "https://en.cppreference.com/w/cpp/container/vector"},
        };

browser_window::browser_window(litehtml::context* html_context) :
        m_html(html_context, this),

        m_tools_render1("Single Render"),
        m_tools_render10("Render 10 Times"),
        m_tools_render100("Render 100 Times"),

        m_tools_draw1("Single Draw"),
        m_tools_draw10("Draw 10 Times"),
        m_tools_draw100("Draw 100 Times"),

        m_tools_dump("Dump parsed HTML")
{
	set_title("litehtml");

	add(m_vbox);
	m_vbox.show();

	m_vbox.pack_start(m_hbox, Gtk::PACK_SHRINK);
	m_hbox.show();

    m_hbox.pack_start(m_back_button, Gtk::PACK_SHRINK);
    m_back_button.show();
    m_back_button.signal_clicked().connect( sigc::mem_fun(*this, &browser_window::on_back_clicked) );
    m_back_button.set_image_from_icon_name("go-previous-symbolic", Gtk::ICON_SIZE_BUTTON);

    m_hbox.pack_start(m_forward_button, Gtk::PACK_SHRINK);
    m_forward_button.show();
    m_forward_button.signal_clicked().connect( sigc::mem_fun(*this, &browser_window::on_forward_clicked) );
    m_forward_button.set_image_from_icon_name("go-next-symbolic", Gtk::ICON_SIZE_BUTTON);

    m_hbox.pack_start(m_address_bar, Gtk::PACK_EXPAND_WIDGET);
	m_address_bar.show();
	m_address_bar.set_text("http://www.litehtml.com/");

	m_address_bar.add_events(Gdk::KEY_PRESS_MASK);
	m_address_bar.signal_key_press_event().connect( sigc::mem_fun(*this, &browser_window::on_address_key_press), false );

    m_go_button.signal_clicked().connect( sigc::mem_fun(*this, &browser_window::on_go_clicked) );

	m_hbox.pack_start(m_go_button, Gtk::PACK_SHRINK);
	m_go_button.show();
    m_go_button.set_image_from_icon_name("media-playback-start-symbolic", Gtk::ICON_SIZE_BUTTON);

    m_menu_bookmarks.set_halign(Gtk::ALIGN_END);

    for(const auto& url : g_bookmarks)
    {
        m_menu_items.emplace_back(url.name);
        m_menu_bookmarks.append(m_menu_items.back());
        m_menu_items.back().signal_activate().connect(
                sigc::bind(
                        sigc::mem_fun(*this, &browser_window::open_url),
                        litehtml::tstring(url.url)));
    }
    m_menu_bookmarks.show_all();

    m_hbox.pack_start(m_bookmarks_button, Gtk::PACK_SHRINK);
    m_bookmarks_button.set_popup(m_menu_bookmarks);
    m_bookmarks_button.show();
    m_bookmarks_button.set_image_from_icon_name("user-bookmarks-symbolic", Gtk::ICON_SIZE_BUTTON);

    m_hbox.pack_start(m_tools_button, Gtk::PACK_SHRINK);
    m_tools_button.set_popup(m_menu_tools);
    m_tools_button.show();
    m_tools_button.set_image_from_icon_name("preferences-system-symbolic", Gtk::ICON_SIZE_BUTTON);

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

    set_default_size(1280, 720);
    update_buttons();
}

browser_window::~browser_window()
{

}

void browser_window::on_go_clicked()
{
	litehtml::tstring url = m_address_bar.get_text();
	open_url(url);
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

void browser_window::open_url(const litehtml::tstring &url)
{
    std::string hash;
    std::string s_url = url;

    m_address_bar.set_text(url);

    std::string::size_type hash_pos = s_url.find_first_of(L'#');
    if(hash_pos != std::wstring::npos)
    {
        hash = s_url.substr(hash_pos + 1);
        s_url.erase(hash_pos);
    }

    bool open_hash_only = false;
    bool reload = false;

    auto current_url = m_history.current();
    hash_pos = current_url.find_first_of(L'#');
    if(hash_pos != std::wstring::npos)
    {
        current_url.erase(hash_pos);
    }

    if(!current_url.empty())
    {
        if(m_history.current() != url)
        {
            if (current_url == s_url)
            {
                open_hash_only = true;
            }
        } else
        {
            reload = true;
        }
    }
    if(!open_hash_only)
    {
        m_html.open_page(url, hash);
    } else
    {
        m_html.show_hash(hash);
    }
    if(!reload)
    {
        m_history.url_opened(url);
    }
    update_buttons();
}

void browser_window::set_url(const litehtml::tstring &url)
{
	m_address_bar.set_text(url);
}

void browser_window::on_forward_clicked()
{
    std::string url;
    if(m_history.forward(url))
    {
        open_url(url);
    }
}

void browser_window::on_back_clicked()
{
    std::string url;
    if(m_history.back(url))
    {
        open_url(url);
    }
}

void browser_window::update_buttons()
{
    std::string url;
    if(m_history.back(url))
    {
        m_back_button.set_state(Gtk::STATE_NORMAL);
    } else
    {
        m_back_button.set_state(Gtk::STATE_INSENSITIVE);
    }
    if(m_history.forward(url))
    {
        m_forward_button.set_state(Gtk::STATE_NORMAL);
    } else
    {
        m_forward_button.set_state(Gtk::STATE_INSENSITIVE);
    }
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
