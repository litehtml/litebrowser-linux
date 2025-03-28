#pragma once

#include <gtkmm.h>
#include "html_host.h"
#include "web_page.h"
#include "http_requests_pool.h"
#include "web_history.h"
#include <queue>

enum page_state
{
	page_state_has_back = 0x01,
	page_state_has_forward = 0x02,
	page_state_downloading = 0x04,
};

class html_widget_notifier : public litebrowser::browser_notify_interface
{
public:
	using redraw_func = std::function<void()>;
	using render_func = std::function<void()>;
	using update_state_func = std::function<void()>;
	using on_page_loaded_func = std::function<void(uint64_t)>;
private:
	enum func_type
	{
		func_type_none,
		func_type_redraw,
		func_type_render,
		func_type_update_state,
		func_type_on_page_loaded
	};
	struct queue_item
	{
		func_type	type;
		uint64_t	param;
	};

	Glib::Dispatcher		m_dispatcher;
	redraw_func				m_redraw_func;
	render_func				m_render_func;
	update_state_func		m_update_state_func;
	on_page_loaded_func		m_on_page_loaded_func;

	std::mutex				m_lock;
	bool					m_disconnect = false;
	std::queue<queue_item>	m_queue;

public:

	html_widget_notifier()
	{
		m_dispatcher.connect(sigc::mem_fun(*this, &html_widget_notifier::on_notify));
	}

	void disconnect()
	{
		std::lock_guard lock(m_lock);
		m_disconnect = true;
	}

	void connect_redraw(redraw_func _redraw_func)
	{
		m_redraw_func = _redraw_func;
	}

	void connect_render(render_func _render_func)
	{
		m_render_func = _render_func;
	}

	void connect_update_state(update_state_func _update_state_func)
	{
		m_update_state_func = _update_state_func;
	}

	void connect_on_page_loaded(on_page_loaded_func _on_page_loaded_func)
	{
		m_on_page_loaded_func = _on_page_loaded_func;
	}

private:

	void redraw() override
	{
		{
			std::lock_guard lock(m_lock);
			m_queue.push(queue_item{func_type_redraw, 0});
		}
		m_dispatcher.emit();
	}

	void render() override
	{
		{
			std::lock_guard lock(m_lock);
			m_queue.push(queue_item{func_type_render, 0});
		}
		m_dispatcher.emit();
	}

	void update_state() override
	{
		{
			std::lock_guard lock(m_lock);
			m_queue.push(queue_item{func_type_update_state, 0});
		}
		m_dispatcher.emit();
	}

	void on_page_loaded(uint64_t web_page_id) override
	{
		{
			std::lock_guard lock(m_lock);
			m_queue.push(queue_item{func_type_on_page_loaded, web_page_id});
		}
		m_dispatcher.emit();
	}

	void on_notify()
	{
		std::queue<queue_item> local_queue;
		{
			std::lock_guard lock(m_lock);
			if(m_disconnect || m_queue.empty()) return;

			while(!m_queue.empty())
			{
				local_queue.push(m_queue.front());
				m_queue.pop();
			}
		}
		func_type prev_type = func_type_none;
		while(!local_queue.empty())
		{
			const auto& item = local_queue.front();
			// We don't need do the same action some times
			if(item.type != prev_type)
			{
				prev_type = item.type;
				switch (item.type)
				{
				case func_type_redraw:
					if(m_redraw_func)
					{
						m_redraw_func();
					}
					break;
				case func_type_render:
					if(m_render_func)
					{
						m_render_func();
					}
					break;
				case func_type_update_state:
					if(m_update_state_func)
					{
						m_update_state_func();
					}
					break;
				case func_type_on_page_loaded:
					if(m_on_page_loaded_func)
					{
						m_on_page_loaded_func(item.param);
					}
					break;

				default:
					break;
				}
			}
			local_queue.pop();
		}
	}
};

class draw_buffer
{
	cairo_surface_t*	m_draw_buffer = nullptr;
	int					m_width = 0;
	int					m_height = 0;
	int					m_top = 0;
	int					m_left = 0;
	double				m_scale_factor = 1;
	int					m_min_int_position = 1;
public:

	~draw_buffer()
	{
		if(m_draw_buffer)
		{
			cairo_surface_destroy(m_draw_buffer);
		}
	}

	[[nodiscard]]
	int get_width() const { return m_width; }

	[[nodiscard]]
	int get_height() const { return m_height; }

	[[nodiscard]]
	int get_left() const { return m_left; }

	[[nodiscard]]
	int get_top() const { return m_top; }

	[[nodiscard]]
	cairo_surface_t* get_cairo_surface() const { return m_draw_buffer; }

	[[nodiscard]]
	double get_scale_factor() const { return m_scale_factor; }

	void set_scale_factor(std::shared_ptr<litebrowser::web_page> page, double scale)
	{
		if(m_scale_factor != scale)
		{
			m_scale_factor = scale;
			m_min_int_position = get_denominator(m_scale_factor);

			m_top = fix_position(m_top);
			m_left = fix_position(m_left);

			if(m_draw_buffer)
			{
				cairo_surface_destroy(m_draw_buffer);
			}
			m_draw_buffer = nullptr;
			create_draw_buffer(m_width, m_height);
			redraw(page);
		}
	}

	bool create_draw_buffer(int width, int height)
	{
		if(m_width != width || m_height != height || !m_draw_buffer)
		{
			m_width = width;
			m_height = height;
			if(m_draw_buffer)
			{
				cairo_surface_destroy(m_draw_buffer);
			}
			m_draw_buffer = nullptr;
			if(m_width > 0 && m_height > 0)
			{
				m_draw_buffer = cairo_image_surface_create(CAIRO_FORMAT_RGB24,
					std::ceil((double) m_width * m_scale_factor),
					std::ceil((double) m_height * m_scale_factor));
			}
		}
		return m_draw_buffer != nullptr;
	}

	void on_size_allocate(std::shared_ptr<litebrowser::web_page> page, int width, int height)
	{
		if(create_draw_buffer(width, height))
		{
			redraw(page);
		}
	}

	void on_scroll(std::shared_ptr<litebrowser::web_page> page, int left, int top)
	{
		if(m_width <= 0 || m_height <= 0 || !m_draw_buffer) return;

		top = fix_position(top);
		left = fix_position(left);

		if(m_left != left || m_top != top)
		{
			Gdk::Rectangle rec_current(m_left, m_top, m_width, m_height); 	// Current area
			Gdk::Rectangle rec_clean(rec_current);							// Clean area
			Gdk::Rectangle rec_new(left, top, m_width, m_height);				// New area
			rec_clean.intersect(rec_new);
			if(rec_clean.has_zero_area() || rec_new == rec_current)
			{
				m_left = left;
				m_top = top;
				redraw(page);
			} else
			{
				int scaled_width  = (int) std::ceil((double) m_width * m_scale_factor);
				int scaled_height = (int) std::ceil((double) m_height * m_scale_factor);
				int surface_shift_x = (int) std::floor((double) (m_left - left) * m_scale_factor);
				int surface_shift_y = (int) std::floor((double) (m_top - top) * m_scale_factor);
				//printf("[surface_shift] top:%d m_top:%d x:%d y:%d\n", top, m_top, surface_shift_x, surface_shift_y);

				auto new_surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24, scaled_width, scaled_height);
				cairo_t* cr = cairo_create(new_surface);
				cairo_rectangle(cr, (rec_clean.get_x() - left) * m_scale_factor - m_scale_factor,
									(rec_clean.get_y() - top) * m_scale_factor - m_scale_factor,
									std::ceil((double) rec_clean.get_width() * m_scale_factor) + 2.0 * m_scale_factor,
									std::ceil((double) rec_clean.get_height() * m_scale_factor) + 2.0 * m_scale_factor);
				cairo_clip(cr);
				cairo_set_source_surface(cr, m_draw_buffer, surface_shift_x, surface_shift_y);
				cairo_paint(cr);
				cairo_destroy(cr);
				cairo_surface_destroy(m_draw_buffer);
				m_draw_buffer = new_surface;

				m_left = left;
				m_top = top;

				int right = m_left + m_width;
				int bottom = m_top + m_height;
				int clean_right = rec_clean.get_x() + rec_clean.get_width();
				int clean_bottom = rec_clean.get_y() + rec_clean.get_height();

				if(rec_clean.get_x() > m_left)
				{
					redraw_area(page, m_left - 1, rec_clean.get_y() - 1, rec_clean.get_x() - m_left + 2, rec_clean.get_height() + 2);
				}
				if(clean_right < right)
				{
					redraw_area(page, clean_right - 1, rec_clean.get_y() - 1, right - clean_right + 2, rec_clean.get_height() + 2);
				}

				if(rec_clean.get_y() > m_top)
				{
					redraw_area(page, 	m_left,
										m_top - 1,
										m_width,
										rec_clean.get_y() - m_top + 2);
				}
				if(clean_bottom < bottom)
				{
					redraw_area(page, 	m_left,
										clean_bottom - 1,
										m_width,
										bottom - clean_bottom + 2);
				}
			}
		}
	}

	void redraw_area(std::shared_ptr<litebrowser::web_page> page, int x, int y, int width, int height)
	{
		if(page && m_draw_buffer)
		{
			// Calculate scaled position
			int s_x = std::floor((double) (x - m_left) * m_scale_factor);
			int s_y = std::floor((double) (y - m_top) * m_scale_factor);
			int s_width = std::ceil((double) width * m_scale_factor);
			int s_height = std::ceil((double) height * m_scale_factor);

			litehtml::position pos {x - m_left, y - m_top, width, height};
			cairo_t* cr = cairo_create(m_draw_buffer);

			// Apply clip with scaled position to avoid artifacts
			cairo_rectangle(cr, s_x, s_y, s_width, s_height);
			cairo_clip(cr);

			cairo_rectangle(cr, s_x, s_y, s_width, s_height);
			cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
			cairo_fill(cr);

			// Apply scale for drawing
			cairo_scale(cr, m_scale_factor, m_scale_factor);

			// Draw page
			page->draw((litehtml::uint_ptr) cr, -m_left, -m_top, &pos);

			cairo_destroy(cr);
		}
	}

	void redraw(std::shared_ptr<litebrowser::web_page> page)
	{
		redraw_area(page, m_left, m_top, m_width, m_height);
	}

private:
	inline int fix_position(int pos)
	{
		return (pos / m_min_int_position) * m_min_int_position;
	}

	static int get_common_divisor(int a, int b)
	{
		while (b != 0)
		{
			int t = b;
			b = a % b;
			a = t;
		}
		return a;
	}

	static int get_denominator(double decimal)
	{
		int numerator = (int)(decimal * 100);
		int denominator = 100;

		int common_divisor = get_common_divisor(numerator, denominator);
		numerator /= common_divisor;
		return denominator / common_divisor;
	}
};

class html_widget :		public Gtk::Widget,
						public litebrowser::html_host_interface
{
	int m_rendered_width = 0;
	std::mutex m_page_mutex;
	std::shared_ptr<litebrowser::web_page> 	m_current_page;
	std::shared_ptr<litebrowser::web_page> 	m_next_page;
	std::shared_ptr<html_widget_notifier>	m_notifier;
	web_history m_history;

	Gtk::Scrollbar*	m_vscrollbar;
	Gtk::Scrollbar*	m_hscrollbar;
	Glib::RefPtr<Gtk::Adjustment>	m_vadjustment;
	Glib::RefPtr<Gtk::Adjustment>	m_hadjustment;

	sigc::connection	m_scrollbar_timer;

	draw_buffer m_draw_buffer;

public:
	explicit html_widget();
	~html_widget() override;

	void on_page_loaded(uint64_t web_page_id);
	void render();
	void go_forward();
	void go_back();
	uint32_t get_state();
	void stop_download();
	void reload();

	std::string get_html_source();
    long render_measure(int number);
    long draw_measure(int number);
	void show_hash(const std::string& hash);
	bool on_close();
	void dump(litehtml::dumper& cout);

	void open_url(const std::string& url) override;

protected:
	// litebrowser::html_host_interface override
	double get_dpi() override;
	int get_screen_width() override;
	int get_screen_height() override;
	void open_page(const litehtml::string& url, const litehtml::string& hash) override;
	void update_cursor() override;
	void redraw_boxes(const litehtml::position::vector& boxes) override;
	int get_render_width() override;
	void scroll_to(int x, int y) override;
	void get_client_rect(litehtml::position& client) const override;
	void set_caption(const char* caption) override;
	cairo_surface_t* load_image(const std::string& path) override;

	void snapshot_vfunc(const Glib::RefPtr<Gtk::Snapshot>& snapshot) override;
	void on_redraw();

	void on_button_press_event(int n_press, double x, double y);
	void on_button_release_event(int n_press, double x, double y);
	void on_mouse_move(double x, double y);
	bool on_key_pressed(guint keyval, guint keycode, Gdk::ModifierType state);

	void size_allocate_vfunc(int width, int height, int baseline) override;
	void allocate_scrollbars(int width, int height);

	void on_vadjustment_changed();
	void on_hadjustment_changed();
	void on_adjustments_changed();
	bool on_scroll(double dx, double dy);
	bool on_scrollbar_timeout();
	void on_realize() override;

private:
	std::shared_ptr<litebrowser::web_page> current_page()
	{
		std::lock_guard<std::mutex> lock(m_page_mutex);
		return m_current_page;
	}

	void update_view_port(std::shared_ptr<litebrowser::web_page> page);
	void restart_scrollbar_timer();
	void force_redraw()
	{
		queue_draw();
		while (g_main_context_iteration(nullptr, false)) {}
	}

public:
	// Signals types
	using sig_set_address_t = sigc::signal<void(std::string)>;
	using sig_update_state_t = sigc::signal<void(uint32_t)>;

	sig_set_address_t signal_set_address() { return  m_sig_set_address; }
	sig_update_state_t signal_update_state() { return  m_sig_update_state; }

private:
	sig_update_state_t m_sig_update_state;
	sig_set_address_t m_sig_set_address;
};
