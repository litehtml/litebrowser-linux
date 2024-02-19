#include "globals.h"
#include "html_widget.h"
#include "browser_wnd.h"
#include <chrono>
#include <ostream>

html_widget::html_widget(browser_window* browser)
{
    m_browser           = browser;
	m_rendered_width	= 0;
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

	{
		auto page = current_page();
		if (page)
		{
			page->draw((litehtml::uint_ptr) cr->cobj(), 0, 0, &pos);
		}
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

void html_widget::set_caption(const char* caption)
{
	if(get_parent_window())
	{
		get_parent_window()->set_title(caption);
	}
}

Gtk::Allocation html_widget::get_parent_allocation()
{
    Gtk::Container* parent = get_parent();
    return parent->get_allocation();
}

void html_widget::open_page(const litehtml::string& url, const litehtml::string& hash)
{
	{
		std::lock_guard<std::mutex> lock(m_page_mutex);
		if (m_current_page)
		{
			m_current_page->stop_loading();
		}
		m_next_page = std::make_shared<litebrowser::web_page>(this, 10);
		m_next_page->open(url, hash);
	}
	m_browser->set_address(url);
	m_browser->update_buttons();
}

void html_widget::scroll_to(int x, int y)
{
    auto vadj = m_browser->get_scrolled()->get_vadjustment();
    auto hadj = m_browser->get_scrolled()->get_hadjustment();
    vadj->set_value(vadj->get_lower() + y);
    hadj->set_value(hadj->get_lower() + x);
}

void html_widget::on_parent_size_allocate(Gtk::Allocation allocation)
{
	std::shared_ptr<litebrowser::web_page> page = current_page();
    if(page && m_rendered_width != allocation.get_width())
    {
        m_rendered_width = allocation.get_width();
        page->media_changed();
		render();
    }
}

void html_widget::on_parent_changed(Gtk::Widget* /*previous_parent*/)
{
    Gtk::Widget* viewport = get_parent();
    if(viewport)
    {
        viewport->signal_size_allocate().connect(sigc::mem_fun(*this, &html_widget::on_parent_size_allocate));
    }
}

bool html_widget::on_button_press_event(GdkEventButton *event)
{
	std::shared_ptr<litebrowser::web_page> page = current_page();
	if(page)
	{
		page->on_lbutton_down((int) event->x, (int) event->y, (int) event->x, (int) event->y);
	}
    return true;
}

bool html_widget::on_button_release_event(GdkEventButton *event)
{
	std::shared_ptr<litebrowser::web_page> page = current_page();
	if(page)
	{
		page->on_lbutton_up((int) event->x, (int) event->y, (int) event->x, (int) event->y);
	}
    return true;
}

bool html_widget::on_motion_notify_event(GdkEventMotion *event)
{
	std::shared_ptr<litebrowser::web_page> page = current_page();
	if(page)
	{
		page->on_mouse_over((int) event->x, (int) event->y, (int) event->x, (int) event->y);
	}
	return true;
}

void html_widget::update_cursor()
{
    Gdk::CursorType cursType = Gdk::ARROW;
	std::shared_ptr<litebrowser::web_page> page = current_page();
	if(page)
	{
		if (page->get_cursor() == "pointer")
		{
			cursType = Gdk::HAND2;
		}
	}
    if(cursType == Gdk::ARROW)
    {
        get_window()->set_cursor();
    } else
    {
        get_window()->set_cursor( Gdk::Cursor::create(cursType) );
    }
}

long html_widget::draw_measure(int number)
{
	std::shared_ptr<litebrowser::web_page> page = current_page();

	if(page)
	{
		auto vadj = m_browser->get_scrolled()->get_vadjustment();
		auto hadj = m_browser->get_scrolled()->get_hadjustment();

		int width = (int) hadj->get_page_size();
		int height = (int) vadj->get_page_size();

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

		int x = (int) (hadj->get_value() - hadj->get_lower());
		int y = (int) (vadj->get_value() - vadj->get_lower());

		cairo_rectangle(cr, 0, 0, width, height);
		cairo_set_source_rgb(cr, 1, 1, 1);
		cairo_paint(cr);
		page->draw((litehtml::uint_ptr) cr, -x, -y, &pos);
		cairo_surface_write_to_png(surface, "/tmp/litebrowser.png");

		auto t1 = std::chrono::high_resolution_clock::now();
		for (int i = 0; i < number; i++)
		{
			page->draw((litehtml::uint_ptr) cr, -x, -y, &pos);
		}
		auto t2 = std::chrono::high_resolution_clock::now();

		cairo_destroy(cr);
		cairo_surface_destroy(surface);
		g_free(image);

		return (std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1)).count();
	}
	return 0;
}

long html_widget::render_measure(int number)
{
	std::shared_ptr<litebrowser::web_page> page = current_page();

    if(page)
    {
        auto t1 = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < number; i++)
        {
			page->render(m_rendered_width);
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        return (std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1)).count();
    }
    return -1;
}

void html_widget::on_size_allocate(Gtk::Allocation& allocation)
{
    Gtk::DrawingArea::on_size_allocate(allocation);
	std::shared_ptr<litebrowser::web_page> page = current_page();

    if(page)
    {
		page->show_hash_and_reset();
    }
}

static int action_redraw(void* data)
{
	auto ctl = (Gtk::Widget*) data;
	ctl->queue_draw();
	return FALSE;
}

static int action_render(void* data)
{
	auto ctl = (html_widget*) data;
	ctl->render();
	return FALSE;
}

static int action_update_state(void* data)
{
	auto ctl = (html_widget*) data;
	ctl->browser()->update_buttons();
	return FALSE;
}

void html_widget::redraw()
{
	g_idle_add(action_redraw, this);
}

void html_widget::redraw_rect(int x, int y, int width, int height)
{
	queue_draw_area(x, y, width, height);
}

int html_widget::get_render_width()
{
	return get_parent_allocation().get_width();
}

void html_widget::on_page_loaded()
{
	std::string url;
	{
		std::lock_guard<std::mutex> lock(m_page_mutex);
		m_current_page = m_next_page;
		m_next_page = nullptr;
		url = m_current_page->url();
		set_size_request(m_current_page->width(), m_current_page->height());
	}
	scroll_to(0, 0);
	redraw();
	m_browser->update_buttons();
	m_browser->set_address(url);
}

void html_widget::show_hash(const std::string &hash)
{
	std::shared_ptr<litebrowser::web_page> page = current_page();
	if(page)
	{
		page->show_hash(hash);
	}
}

void html_widget::dump(const std::string &file_name)
{
	std::shared_ptr<litebrowser::web_page> page = current_page();
	if(page)
	{
		page->dump(file_name);
	}
}

void html_widget::open_url(const std::string &url)
{
	std::string hash;
	std::string s_url = url;

	m_browser->set_address(url);

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
		open_page(url, hash);
	} else
	{
		show_hash(hash);
	}
	if(!reload)
	{
		m_history.url_opened(url);
	}
	m_browser->update_buttons();
}

void html_widget::render()
{
	std::shared_ptr<litebrowser::web_page> page = current_page();
	if(page)
	{
		page->render(m_rendered_width);
		set_size_request(page->width(), page->height());
		queue_draw();
	}
}

bool html_widget::on_close(GdkEventAny* /*event*/)
{
	if(m_current_page)
	{
		m_current_page->stop_loading();
	}
	if(m_next_page)
	{
		m_next_page->stop_loading();
	}
	return false;
}

void html_widget::go_forward()
{
	std::string url;
	if(m_history.forward(url))
	{
		open_url(url);
	}
}

void html_widget::go_back()
{
	std::string url;
	if(m_history.back(url))
	{
		open_url(url);
	}
}

uint32_t html_widget::get_state()
{
	uint32_t ret = 0;
	std::string url;
	if(m_history.back(url))
	{
		ret |= page_state_has_back;
	}
	if(m_history.forward(url))
	{
		ret |= page_state_has_forward;
	}
	{
		std::lock_guard<std::mutex> lock(m_page_mutex);
		if(m_next_page)
		{
			ret |= page_state_downloading;
		}
	}
	if(!(ret & page_state_downloading))
	{
		std::shared_ptr<litebrowser::web_page> page = current_page();
		if(page)
		{
			if(page->is_downloading())
			{
				ret |= page_state_downloading;
			}
		}
	}
	return ret;
}

void html_widget::queue_action(litebrowser::html_host_interface::q_action action)
{
	switch (action)
	{
		case queue_action_redraw:
			g_idle_add(action_redraw, this);
			break;
		case queue_action_render:
			g_idle_add(action_render, this);
			break;
		case queue_action_update_state:
			g_idle_add(action_update_state, this);
			break;
	}
}

void html_widget::stop_download()
{
	std::lock_guard<std::mutex> lock(m_page_mutex);
	if(m_next_page)
	{
		m_next_page->stop_loading();
	} else if (m_current_page)
	{
		m_current_page->stop_loading();
	}
}

void html_widget::reload()
{
	std::shared_ptr<litebrowser::web_page> page = current_page();
	if(page)
	{
		open_url(page->url());
	}
}
