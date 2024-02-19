#pragma once

#include <gtkmm/drawingarea.h>
#include <thread>
#include "../litehtml/containers/linux/container_linux.h"
#include "html_host.h"
#include "web_page.h"
#include "http_requests_pool.h"
#include "web_history.h"

class browser_window;

enum page_state
{
	page_state_has_back = 0x01,
	page_state_has_forward = 0x02,
	page_state_downloading = 0x04,
};

class html_widget :		public Gtk::DrawingArea,
						public litebrowser::html_host_interface
{
	int m_rendered_width;
	browser_window* m_browser;
	std::mutex m_page_mutex;
	std::shared_ptr<litebrowser::web_page> m_current_page;
	std::shared_ptr<litebrowser::web_page> m_next_page;
	web_history m_history;
public:
	explicit html_widget(browser_window* browser);
	~html_widget() override;

	void open_page(const litehtml::string& url, const litehtml::string& hash) override;
	void update_cursor() override;
	void on_parent_size_allocate(Gtk::Allocation allocation);
    void on_size_allocate(Gtk::Allocation& allocation) override;
	void redraw() override;
	void redraw_rect(int x, int y, int width, int height) override;
	int get_render_width() override;
	void on_page_loaded() override;
	void dump(const litehtml::string& file_name);
	void open_url(const std::string& url) override;
	void render() override;
	void queue_action(litebrowser::html_host_interface::q_action action) override;
	void go_forward();
	void go_back();
	uint32_t get_state();
	void stop_download();
	void reload();
	browser_window* browser() const { return m_browser; }

    long render_measure(int number);
    long draw_measure(int number);
	void show_hash(const std::string& hash);
	bool on_close(GdkEventAny* event);

protected:
	bool on_draw(const Cairo::RefPtr<Cairo::Context>& cr) override;
	void scroll_to(int x, int y) override;

	void get_client_rect(litehtml::position& client) const override;
	void set_caption(const char* caption) override;

	bool on_button_press_event(GdkEventButton* event) override;
	bool on_button_release_event(GdkEventButton* event) override;
	bool on_motion_notify_event(GdkEventMotion* event) override;

	void on_parent_changed(Gtk::Widget* previous_parent) override;

private:
	std::shared_ptr<litebrowser::web_page> current_page()
	{
		std::lock_guard<std::mutex> lock(m_page_mutex);
		return m_current_page;
	}
	Gtk::Allocation get_parent_allocation();
};
