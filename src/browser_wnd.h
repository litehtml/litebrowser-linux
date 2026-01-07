#pragma once

#include "html_widget.h"
#include <string>

class browser_window : public Gtk::Window
{
  public:
	browser_window(Gio::Application* app, const std::string& url);
	~browser_window() override;

	void update_buttons(uint32_t);
	void set_address(const std::string& text) { m_address_bar.set_text(text); }

  private:
	void on_go_clicked();
	void on_forward_clicked();
	void on_stop_reload_clicked();
	void on_home_clicked();
	void on_back_clicked();
	void on_render_measure(int number);
	void on_draw_measure(int number);
	void on_dump();
	void on_test_append_children_from_string(bool replace_existing);

  protected:
	uint32_t							m_prev_state = 0;
	html_widget							m_html;
	Gtk::Entry							m_address_bar;
	Gtk::Button							m_go_button;
	Gtk::Button							m_forward_button;
	Gtk::Button							m_back_button;
	Gtk::Button							m_stop_reload_button;
	Gtk::Button							m_home_button;
	Gtk::Button							m_bookmarks_button;
	Gtk::Button							m_tools_button;
	Gtk::HeaderBar						m_header;
	Gtk::PopoverMenu					m_bookmarks_popover;
	Gtk::PopoverMenu					m_tools_popover;

	std::unique_ptr<Gtk::MessageDialog> m_pDialog;
};
