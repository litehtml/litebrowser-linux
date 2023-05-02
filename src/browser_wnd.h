#pragma once

#include "html_widget.h"
#include <vector>
#include <string>
#include "web_history.h"

class browser_window : public Gtk::Window
{
public:
	browser_window();
	virtual ~browser_window();

	void open_url(const litehtml::string& url);
	void set_url(const litehtml::string& url);

    Gtk::ScrolledWindow* get_scrolled() { return &m_scrolled_wnd; }

private:
    void on_go_clicked();
    void on_forward_clicked();
    void on_back_clicked();
    void on_render_measure(int number);
    void on_draw_measure(int number);
    bool on_address_key_press(GdkEventKey* event);
    void on_dump();

protected:
	html_widget			m_html;
	Gtk::Entry			m_address_bar;
    Gtk::Button			m_go_button;
    Gtk::Button			m_forward_button;
    Gtk::Button			m_back_button;
    Gtk::MenuButton		m_bookmarks_button;
    Gtk::MenuButton		m_tools_button;
	Gtk::VBox			m_vbox;
	Gtk::HeaderBar		m_header;
	Gtk::ScrolledWindow m_scrolled_wnd;
    web_history         m_history;

    Gtk::Menu           m_menu_bookmarks;
    std::vector<Gtk::MenuItem> m_menu_items;

    Gtk::Menu           m_menu_tools;
    Gtk::MenuItem       m_tools_render1;
    Gtk::MenuItem       m_tools_render10;
    Gtk::MenuItem       m_tools_render100;
    Gtk::MenuItem       m_tools_draw1;
    Gtk::MenuItem       m_tools_draw10;
    Gtk::MenuItem       m_tools_draw100;
    Gtk::MenuItem       m_tools_dump;

    std::unique_ptr<Gtk::MessageDialog> m_pDialog;

    void update_buttons();
};

