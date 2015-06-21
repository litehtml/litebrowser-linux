#pragma once

#include <gtkmm/drawingarea.h>
#include "../litehtml/containers/linux/container_linux.h"
#include "http_loader.h"

class browser_window;

class html_widget :		public Gtk::DrawingArea,
						public container_linux
{
	litehtml::tstring			m_url;
	litehtml::tstring			m_base_url;
	litehtml::document::ptr		m_html;
	litehtml::context*			m_html_context;
	int							m_rendered_width;
	litehtml::tstring			m_cursor;
	litehtml::tstring			m_clicked_url;
	browser_window*				m_browser;
	http_loader					m_http;
public:
	html_widget(litehtml::context* html_context, browser_window* browser);
	virtual ~html_widget();

	void open_page(const litehtml::tstring& url);
	void update_cursor();

protected:
	virtual bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr);

	virtual void get_client_rect(litehtml::position& client) const;
	virtual	 void on_anchor_click(const litehtml::tchar_t* url, litehtml::element::ptr el);
	virtual	 void set_cursor(const litehtml::tchar_t* cursor);
	virtual void import_css(litehtml::tstring& text, const litehtml::tstring& url, litehtml::tstring& baseurl);
	virtual	 void set_caption(const litehtml::tchar_t* caption);
	virtual	 void set_base_url(const litehtml::tchar_t* base_url);
	virtual	 void link(litehtml::document* doc, litehtml::element::ptr el);
	virtual Glib::RefPtr<Gdk::Pixbuf>	get_image(const litehtml::tchar_t* url, bool redraw_on_ready);
	virtual void						make_url( const litehtml::tchar_t* url, const litehtml::tchar_t* basepath, litehtml::tstring& out );

    virtual void get_preferred_width_vfunc(int& minimum_width, int& natural_width) const;
    virtual void get_preferred_height_vfunc(int& minimum_height, int& natural_height) const;

    virtual void on_size_allocate(Gtk::Allocation& allocation);
	virtual bool on_button_press_event(GdkEventButton* event);
	virtual bool on_button_release_event(GdkEventButton* event);
	virtual bool on_motion_notify_event(GdkEventMotion* event);

private:
	void load_text_file(const litehtml::tstring& url, litehtml::tstring& out);
};
