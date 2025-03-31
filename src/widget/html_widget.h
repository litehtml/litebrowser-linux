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
	using on_set_caption_func = std::function<void(const std::string&)>;
private:
	enum func_type
	{
		func_type_none,
		func_type_redraw,
		func_type_render,
		func_type_update_state,
		func_type_on_page_loaded,
		func_type_on_set_caption
	};
	struct queue_item
	{
		func_type	type;
		uint64_t	param;
		std::string	str_param;
	};

	Glib::Dispatcher		m_dispatcher;
	redraw_func				m_redraw_func;
	render_func				m_render_func;
	update_state_func		m_update_state_func;
	on_page_loaded_func		m_on_page_loaded_func;
	on_set_caption_func		m_on_set_caption_func;

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

	void connect_on_set_caption(on_set_caption_func _on_set_caption_func)
	{
		m_on_set_caption_func = _on_set_caption_func;
	}

private:

	void redraw() override
	{
		{
			std::lock_guard lock(m_lock);
			m_queue.push(queue_item{func_type_redraw, 0, {}});
		}
		m_dispatcher.emit();
	}

	void render() override
	{
		{
			std::lock_guard lock(m_lock);
			m_queue.push(queue_item{func_type_render, 0, {}});
		}
		m_dispatcher.emit();
	}

	void update_state() override
	{
		{
			std::lock_guard lock(m_lock);
			m_queue.push(queue_item{func_type_update_state, 0, {}});
		}
		m_dispatcher.emit();
	}

	void on_page_loaded(uint64_t web_page_id) override
	{
		{
			std::lock_guard lock(m_lock);
			m_queue.push(queue_item{func_type_on_page_loaded, web_page_id, {}});
		}
		m_dispatcher.emit();
	}

	void on_set_caption(const std::string& caption) override
	{
		{
			std::lock_guard lock(m_lock);
			m_queue.push(queue_item{func_type_on_set_caption, 0, caption});
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
				case func_type_on_set_caption:
					if(m_on_set_caption_func)
					{
						m_on_set_caption_func(item.str_param);
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

/// @brief Draw Buffer Class
///
/// This class performs the draw operations into the cairo surface.
/// The application draws everything to the buffer, then buffer are
/// drawn on widged or window.
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

	/// @brief Set scale factor for draw buffer
	/// @param page the webpage to be redraw if required
	/// @param scale the scale factor to be applied
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

	/// @brief Create cairo surface for draw buffer
	/// @param width surface width (not scaled)
	/// @param height surface height (not scaled)
	/// @param scale_factor scale factor
	/// @return poiter to the cairo surface
	cairo_surface_t* make_surface(int width, int height, double scale_factor)
	{
		return cairo_image_surface_create(CAIRO_FORMAT_RGB24,
			std::ceil((double) width * scale_factor),
			std::ceil((double) height * scale_factor));
	}

	/// @brief Creates new buffer with specified size
	/// @param width draw buffer width (not scaled)
	/// @param height draw buffer height (not scaled)
	/// @return true if new draw buffer was created, false if the old buffer was used
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
				m_draw_buffer = make_surface(m_width, m_height, m_scale_factor);
			}
			return true;
		}
		return false;
	}

	/// @brief Call this function when widget size changed
	/// @param page webpage to be redraw if buffer size changed
	/// @param width new draw buffer width
	/// @param height new draw buffer height
	void on_size_allocate(std::shared_ptr<litebrowser::web_page> page, int width, int height)
	{
		if(create_draw_buffer(width, height))
		{
			redraw(page);
		}
	}

	/// @brief Scrolls draw buffer to the position (left, top).
	///
	/// Note, the actual position of the draw buffer can be rounded according to the scale factor.
	/// Use get_left() and get_top() to know the actual position.
	///
	/// @param page webpage to be redraw if the position was changed
	/// @param left new horizontal position
	/// @param top new vertical position
	void on_scroll(std::shared_ptr<litebrowser::web_page> page, int left, int top)
	{
		if(m_width <= 0 || m_height <= 0 || !m_draw_buffer) return;

		top = fix_position(top);
		left = fix_position(left);

		if(m_left != left || m_top != top)
		{
			Gdk::Rectangle rec_current(m_left, m_top, m_width, m_height); 	// Current area
			Gdk::Rectangle rec_clean(rec_current);							// Clean area
			Gdk::Rectangle rec_new(left, top, m_width, m_height);			// New area
			rec_clean.intersect(rec_new);
			if(rec_clean.has_zero_area() || rec_new == rec_current)
			{
				m_left = left;
				m_top = top;
				redraw(page);
			} else
			{
				int surface_shift_x = (int) std::floor((double) (m_left - left) * m_scale_factor);
				int surface_shift_y = (int) std::floor((double) (m_top - top) * m_scale_factor);

				auto new_surface = make_surface(m_width, m_height, m_scale_factor);
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

				int right = fix_position(m_left + m_width);
				int bottom = fix_position(m_top + m_height);
				int clean_right = fix_position(rec_clean.get_x() + rec_clean.get_width());
				int clean_bottom = fix_position(rec_clean.get_y() + rec_clean.get_height());

				if(rec_clean.get_x() > m_left)
				{
					redraw_area(page, 	m_left,
										rec_clean.get_y(),
										rec_clean.get_x() - m_left,
										rec_clean.get_height());
				}
				if(clean_right < right)
				{
					redraw_area(page, 	clean_right,
										rec_clean.get_y(),
										right - clean_right,
										rec_clean.get_height());
				}

				if(rec_clean.get_y() > m_top)
				{
					redraw_area(page, 	m_left,
										m_top,
										m_width,
										rec_clean.get_y() - m_top);
				}
				if(clean_bottom < bottom)
				{
					redraw_area(page, 	m_left,
										clean_bottom,
										m_width,
										bottom - clean_bottom);
				}
			}
		}
	}

	/// @brief Reraw the defined area of the buffer
	///
	/// All coordinated are not scaled. Actual rectangle could be different according to the scale factor,
	/// but it must always cover the requested.
	///
	/// @param page webpage to be redraw
	/// @param x left position of the area
	/// @param y top position of the area
	/// @param width width of the area
	/// @param height height of the area
	void redraw_area(std::shared_ptr<litebrowser::web_page> page, int x, int y, int width, int height)
	{
		if(m_draw_buffer)
		{
			int fixed_left 		= fix_position(x - m_left);
			int fixed_right 	= fix_position(x - m_left + width);
			int fixed_top 		= fix_position(y - m_top);
			int fixed_bottom 	= fix_position(y - m_top + height);

			if(fixed_right < x + width) fixed_right += m_min_int_position;
			if(fixed_bottom < y + height) fixed_bottom += m_min_int_position;

			int fixed_x 		= fixed_left;
			int fixed_y 		= fixed_top;
			int fixed_width 	= fixed_right - fixed_left;
			int fixed_height 	= fixed_bottom - fixed_top;

			int s_x = (int) std::round((double) fixed_x * m_scale_factor);
			int s_y = (int) std::round((double) fixed_y * m_scale_factor);
			int s_width = (int) std::round((double) fixed_width * m_scale_factor);
			int s_height = (int) std::round((double) fixed_height * m_scale_factor);

			litehtml::position pos {fixed_x, fixed_y, fixed_width, fixed_height};
			cairo_t* cr = cairo_create(m_draw_buffer);

			// Apply clip with scaled position to avoid artifacts
			cairo_rectangle(cr, s_x, s_y, s_width, s_height);
			cairo_clip(cr);

			// Clear rectangle with scaled position
			cairo_rectangle(cr, s_x, s_y, s_width, s_height);
			cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
			cairo_fill(cr);

			// Apply scale for drawing
			cairo_scale(cr, m_scale_factor, m_scale_factor);

			// Draw page
			if(page)
			{
				page->draw((litehtml::uint_ptr) cr, -m_left, -m_top, &pos);
			}

			cairo_destroy(cr);
		}
	}

	/// @brief Redraw entire buffer
	/// @param page webpage to be redraw
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
	int m_rendered_height = 0;
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
	cairo_surface_t* load_image(const std::string& path) override;

	void snapshot_vfunc(const Glib::RefPtr<Gtk::Snapshot>& snapshot) override;
	void on_redraw();

	void on_button_press_event(int n_press, double x, double y);
	void on_button_release_event(int n_press, double x, double y);
	void on_mouse_move(double x, double y);
	bool on_key_pressed(guint keyval, guint keycode, Gdk::ModifierType state);

	void size_allocate_vfunc(int width, int height, int baseline) override;
	void allocate_scrollbars(int width, int height);

	void set_caption(const std::string& caption);
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
