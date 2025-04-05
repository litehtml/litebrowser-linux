#pragma once

#include "html_widget.h"
#include <vector>
#include <string>
#include "web_history.h"

class browser_window : public Gtk::Window
{
public:
	browser_window(const Glib::RefPtr<Gio::Application>& app, const std::string& url);
	~browser_window() override;

	void update_buttons(uint32_t);
	void set_address(const std::string& text)
	{
		m_address_bar.set_text(text);
	}

private:
    void on_go_clicked();
	void on_forward_clicked();
	void on_stop_reload_clicked();
	void on_home_clicked();
    void on_back_clicked();
    void on_render_measure(int number);
    void on_draw_measure(int number);
    void on_dump();

protected:
	uint32_t            m_prev_state;
	html_widget			m_html;
	Gtk::Entry			m_address_bar;
    Gtk::Button			m_go_button;
    Gtk::Button			m_forward_button;
	Gtk::Button			m_back_button;
	Gtk::Button			m_stop_reload_button;
	Gtk::Button			m_home_button;
    Gtk::Button		    m_bookmarks_button;
    Gtk::Button		    m_tools_button;
	Gtk::Box			m_vbox {Gtk::Orientation::HORIZONTAL};
	Gtk::HeaderBar		m_header;
    Gtk::PopoverMenu    m_bookmarks_popover;
    Gtk::PopoverMenu    m_tools_popover;

    std::unique_ptr<Gtk::MessageDialog> m_pDialog;

};

