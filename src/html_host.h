#ifndef LITEBROWSER_HTML_HOST_H
#define LITEBROWSER_HTML_HOST_H

#include <litehtml.h>

namespace litebrowser
{
	class html_host_interface
	{
	public:
		enum q_action
		{
			queue_action_redraw,
			queue_action_render,
			queue_action_update_state,
		};

		virtual void open_url(const std::string& url) = 0;
		virtual void open_page(const litehtml::string& url, const litehtml::string& hash) = 0;
		virtual void update_cursor() = 0;
		virtual void scroll_to(int x, int y) = 0;
		virtual void get_client_rect(litehtml::position& client) const = 0;
		virtual void set_caption(const char* caption) = 0;
		virtual void redraw_rect(int x, int y, int width, int height) = 0;
		virtual void redraw() = 0;
		virtual void render() = 0;
		virtual int get_render_width() = 0;
		virtual void on_page_loaded() = 0;
		virtual void queue_action(q_action action) = 0;
	};
}

#endif //LITEBROWSER_HTML_HOST_H
