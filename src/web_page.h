#ifndef LITEBROWSER_WEB_PAGE_H
#define LITEBROWSER_WEB_PAGE_H

#include "container_cairo_pango.h"
#include <gtkmm.h>
#include "html_host.h"
#include "http_requests_pool.h"
#include "cairo_images_cache.h"

namespace litebrowser
{
	class text_file
	{
		std::mutex wait_mutex;
		std::stringstream stream;
		bool data_ready;
	public:
		text_file() : data_ready(false)
		{
			wait_mutex.lock();
		}

		void set_ready() { data_ready = true; }

		std::string str() const { return data_ready ? stream.str() : ""; }
		void wait()
		{
			wait_mutex.lock();
		}
		void on_data(void* data, size_t len, size_t downloaded, size_t total);
		void on_page_downloaded(u_int32_t http_status, u_int32_t err_code, const std::string& err_text);
	};

	class image_file
	{
		int m_fd;
		std::string m_path;
		std::string m_url;
		bool m_redraw_on_ready;
	public:
		explicit image_file(std::string url, bool redraw_on_ready);
		void on_data(void* data, size_t len, size_t downloaded, size_t total);
		void close() const
		{
			if(m_fd > 0)
			{
				::close(m_fd);
			}
		}
		const std::string& path() const { return m_path; }
		const std::string& url() const { return m_url; }
		bool redraw_only() const { return m_redraw_on_ready; }
	};

	class web_page : 	public container_cairo_pango,
						public std::enable_shared_from_this<web_page>
	{
		litehtml::string			m_url;
		litehtml::string			m_base_url;
		litehtml::document::ptr		m_html;
		std::mutex					m_html_mutex;
		litehtml::string			m_cursor;
		litehtml::string			m_clicked_url;
		std::string                 m_hash;
		html_host_interface*		m_html_host;
		cairo_images_cache			m_images;
		litebrowser::http_requests_pool m_requests_pool;

	public:
		explicit web_page(html_host_interface* html_host, int pool_size) :
				m_html_host(html_host),
				m_requests_pool(pool_size, std::bind(&web_page::on_pool_update_state, this))
		{}

		void open(const litehtml::string& url, const litehtml::string& hash);

		void get_client_rect(litehtml::position& client) const override;
		void on_anchor_click(const char* url, const litehtml::element::ptr& el) override;
		void on_mouse_event(const litehtml::element::ptr& el, litehtml::mouse_event event) override;
		void set_cursor(const char* cursor) override;
		void import_css(litehtml::string& text, const litehtml::string& url, litehtml::string& baseurl) override;
		void set_caption(const char* caption) override;
		void set_base_url(const char* base_url) override;
		cairo_surface_t* get_image(const std::string& url) override;
		void make_url( const char* url, const char* basepath, litehtml::string& out ) override;
		void load_image(const char* src, const char* baseurl, bool redraw_on_ready) override;
		static cairo_surface_t* surface_from_pixbuf(const Glib::RefPtr<Gdk::Pixbuf>& bmp);
		double get_screen_dpi() const override;
		int get_screen_width() const override
		{
			return Gdk::screen_width();
		}
		int get_screen_height() const override
		{
			return Gdk::screen_height();
		}

		void show_hash(const litehtml::string& hash);
		void show_hash_and_reset()
		{
			if(!m_hash.empty() && m_html)
			{
				show_hash(m_hash);
				m_hash = "";
			}
		}

		void on_mouse_over(int x, int y, int client_x, int client_y);
		void on_lbutton_down(int x, int y, int client_x, int client_y);
		void on_lbutton_up(int x, int y, int client_x, int client_y);
		const std::string& get_cursor() const	{ return m_cursor; }
		void draw(litehtml::uint_ptr hdc, int x, int y, const litehtml::position* clip)
		{
			std::unique_lock<std::mutex> html_lock(m_html_mutex);
			if(m_html) m_html->draw(hdc, x, y, clip);
		}
		int render(int max_width)
		{
			std::unique_lock<std::mutex> html_lock(m_html_mutex);
			return m_html ? m_html->render(max_width) : 0;
		}
		const std::string& url() const { return m_url; }
		int width() const { return m_html ? m_html->width() : 0; }
		int height() const { return m_html ? m_html->height() : 0; }
		bool media_changed()
		{
			std::unique_lock<std::mutex> html_lock(m_html_mutex);
			return m_html && m_html->media_changed();
		}
		void stop_loading()
		{
			m_requests_pool.stop();
		}
		bool is_downloading()
		{
			return m_requests_pool.is_downloading();
		}
		void dump(const litehtml::string& file_name);
	private:
		void http_request(const std::string& url,
						  const std::function<void(void* data, size_t len, size_t downloaded, size_t total)>& cb_on_data,
						  const std::function<void(u_int32_t http_status, u_int32_t err_code, const std::string& err_text, const std::string& url)>& cb_on_finish);
		void on_page_downloaded(std::shared_ptr<text_file> data, u_int32_t http_status, u_int32_t err_code, const std::string& err_text, const std::string& url);
		void on_image_downloaded(std::shared_ptr<image_file> data, u_int32_t http_status, u_int32_t err_code, const std::string& err_text, const std::string& url);
		void on_pool_update_state();
	};
}

#endif //LITEBROWSER_WEB_PAGE_H
