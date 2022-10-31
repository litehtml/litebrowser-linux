#pragma once

#include <gtkmm/drawingarea.h>
#include "../litehtml/containers/linux/container_linux.h"
#include "http_loader.h"

class browser_window;

class html_widget :		public Gtk::DrawingArea,
						public container_linux
{
	litehtml::string			m_url;
	litehtml::string			m_base_url;
	litehtml::document::ptr		m_html;
	int							m_rendered_width;
	litehtml::string			m_cursor;
	litehtml::string			m_clicked_url;
	browser_window*				m_browser;
	http_loader					m_http;
    std::string                 m_hash;
    bool                        m_hash_valid;
public:
	html_widget(browser_window* browser);
	virtual ~html_widget();

	void open_page(const litehtml::string& url, const litehtml::string& hash);
	void show_hash(const litehtml::string& hash);
    void dump(const litehtml::string& file_name);
	void update_cursor();
	void on_parent_size_allocate(Gtk::Allocation allocation);
    void on_size_allocate(Gtk::Allocation& allocation) override;

    long render_measure(int number);
    long draw_measure(int number);

protected:
	bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;
	void scroll_to(int x, int y);

	void get_client_rect(litehtml::position& client) const override;
	void on_anchor_click(const char* url, const litehtml::element::ptr& el) override;
	void set_cursor(const char* cursor) override;
	void import_css(litehtml::string& text, const litehtml::string& url, litehtml::string& baseurl) override;
	void set_caption(const char* caption) override;
	void set_base_url(const char* base_url) override;
	Glib::RefPtr<Gdk::Pixbuf>	get_image(const char* url, bool redraw_on_ready) override;
	void make_url( const char* url, const char* basepath, litehtml::string& out ) override;

	bool on_button_press_event(GdkEventButton* event) override;
	bool on_button_release_event(GdkEventButton* event) override;
	bool on_motion_notify_event(GdkEventMotion* event) override;

	void on_parent_changed(Gtk::Widget* previous_parent) override;

private:
	void load_text_file(const litehtml::string& url, litehtml::string& out);
	Gtk::Allocation get_parent_allocation();
};
