#include "globals.h"
#include "html_widget.h"
#include "browser_wnd.h"

html_widget::html_widget(litehtml::context* html_context, browser_window* browser)
{
    m_browser           = browser;
	m_rendered_width	= 0;
	m_html_context 		= html_context;
	m_html 				= NULL;
	add_events(Gdk::POINTER_MOTION_MASK | Gdk::BUTTON_PRESS_MASK | Gdk::BUTTON_RELEASE_MASK);
}

html_widget::~html_widget()
{

}

bool html_widget::on_draw(const Cairo::RefPtr<Cairo::Context>& cr)
{
	litehtml::position pos;

    GdkRectangle rect;
    gdk_cairo_get_clip_rectangle(cr->cobj(), &rect);

	pos.width 	= rect.width;
	pos.height 	= rect.height;
	pos.x 		= rect.x;
	pos.y 		= rect.y;

	cr->rectangle(0, 0, get_allocated_width(), get_allocated_height());
	cr->set_source_rgb(1, 1, 1);
	cr->fill();

	if(m_html)
	{
		m_html->draw((litehtml::uint_ptr) cr->cobj(), 0, 0, &pos);
	}

	return true;
}

void html_widget::get_client_rect(litehtml::position& client) const
{
	client.width = get_parent()->get_allocated_width();
	client.height = get_parent()->get_allocated_height();
	client.x = 0;
	client.y = 0;
}


void html_widget::on_anchor_click(const litehtml::tchar_t* url, const litehtml::element::ptr& el)
{
    if(url)
    {
        make_url(url, m_base_url.c_str(), m_clicked_url);
    }
}

void html_widget::set_cursor(const litehtml::tchar_t* cursor)
{
    if(cursor)
    {
        if(m_cursor != cursor)
        {
            m_cursor = cursor;
            update_cursor();
        }
    }
}

void html_widget::import_css(litehtml::tstring& text, const litehtml::tstring& url, litehtml::tstring& baseurl)
{
	std::string css_url;
	make_url(url.c_str(), baseurl.c_str(), css_url);
	load_text_file(css_url, text);
	if(!text.empty())
	{
		baseurl = css_url;
	}
}

void html_widget::set_caption(const litehtml::tchar_t* caption)
{
	if(get_parent_window())
	{
		get_parent_window()->set_title(caption);
	}
}

void html_widget::set_base_url(const litehtml::tchar_t* base_url)
{
	if(base_url)
	{
		m_base_url = urljoin(m_url, std::string(base_url));
	} else
	{
		m_base_url = m_url;
	}
}

Glib::RefPtr<Gdk::Pixbuf> html_widget::get_image(const litehtml::tchar_t* url, bool redraw_on_ready)
{
	Glib::RefPtr< Gio::InputStream > stream = m_http.load_file(url);
	Glib::RefPtr<Gdk::Pixbuf> ptr = Gdk::Pixbuf::create_from_stream(stream);
	return ptr;
}

Gtk::Allocation html_widget::get_parent_allocation()
{
    Gtk::Container* parent = get_parent();
    return parent->get_allocation();
}

void html_widget::open_page(const litehtml::tstring& url)
{
	m_url 		= url;
	m_base_url	= url;

	std::string html;
	load_text_file(url, html);
	m_url 		= m_http.get_url();
	m_base_url	= m_http.get_url();
	m_browser->set_url(m_url);
	m_html = litehtml::document::createFromString(html.c_str(), this, m_html_context);
	if(m_html)
	{
		m_rendered_width = get_parent_allocation().get_width();
		m_html->render(m_rendered_width);
		set_size_request(m_html->width(), m_html->height());
	}

    queue_draw();
}

void html_widget::make_url(const litehtml::tchar_t* url, const litehtml::tchar_t* basepath, litehtml::tstring& out)
{
	if(!basepath || (basepath && !basepath[0]))
	{
		if(!m_base_url.empty())
		{
			out = urljoin(m_base_url, std::string(url));
		} else
		{
			out = url;
		}
	} else
	{
		out = urljoin(std::string(basepath), std::string(url));
	}
}

void html_widget::on_parent_size_allocate(Gtk::Allocation allocation)
{
    if(m_html && m_rendered_width != allocation.get_width())
    {
        m_rendered_width = allocation.get_width();
        m_html->media_changed();
        m_html->render(m_rendered_width);
        set_size_request(m_html->width(), m_html->height());
        queue_draw();
    }
}

void html_widget::on_parent_changed(Gtk::Widget* previous_parent)
{
    Gtk::Widget* viewport = get_parent();
    if(viewport)
    {
        viewport->signal_size_allocate().connect(sigc::mem_fun(*this, &html_widget::on_parent_size_allocate));
    }

}

bool html_widget::on_button_press_event(GdkEventButton *event)
{
    if(m_html)
    {
        litehtml::position::vector redraw_boxes;
        if(m_html->on_lbutton_down((int) event->x, (int) event->y, (int) event->x, (int) event->y, redraw_boxes))
        {
            for(auto& pos : redraw_boxes)
            {
                queue_draw_area(pos.x, pos.y, pos.width, pos.height);
            }
        }
    }
    return true;
}

bool html_widget::on_button_release_event(GdkEventButton *event)
{
    if(m_html)
    {
        litehtml::position::vector redraw_boxes;
		m_clicked_url.clear();
        if(m_html->on_lbutton_up((int) event->x, (int) event->y, (int) event->x, (int) event->y, redraw_boxes))
        {
            for(auto& pos : redraw_boxes)
            {
                queue_draw_area(pos.x, pos.y, pos.width, pos.height);
            }
        }
		if(!m_clicked_url.empty())
		{
			m_browser->open_url(m_clicked_url);
		}
    }
    return true;
}

bool html_widget::on_motion_notify_event(GdkEventMotion *event)
{
    if(m_html)
    {
        litehtml::position::vector redraw_boxes;
        if(m_html->on_mouse_over((int) event->x, (int) event->y, (int) event->x, (int) event->y, redraw_boxes))
        {
            for(auto& pos : redraw_boxes)
            {
                queue_draw_area(pos.x, pos.y, pos.width, pos.height);
            }
        }
    }
	return true;
}

void html_widget::update_cursor()
{
    Gdk::CursorType cursType = Gdk::ARROW;
    if(m_cursor == _t("pointer"))
    {
        cursType = Gdk::HAND1;
    }
    if(cursType == Gdk::ARROW)
    {
        get_window()->set_cursor();
    } else
    {
        get_window()->set_cursor( Gdk::Cursor::create(cursType) );
    }
}

void html_widget::load_text_file(const litehtml::tstring& url, litehtml::tstring& out)
{
    out.clear();
    Glib::RefPtr< Gio::InputStream > stream = m_http.load_file(url);
    gssize sz;
    char buff[1025];
    while( (sz = stream->read(buff, 1024)) > 0 )
    {
        buff[sz] = 0;
        out += buff;
    }
}
