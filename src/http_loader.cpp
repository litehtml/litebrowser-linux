#include "globals.h"
#include "http_loader.h"

http_loader::http_loader()
{
    m_curl = curl_easy_init();
    curl_easy_setopt(m_curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(m_curl, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(m_curl, CURLOPT_TCP_KEEPIDLE, 120L);
    curl_easy_setopt(m_curl, CURLOPT_TCP_KEEPINTVL, 60L);
    curl_easy_setopt(m_curl, CURLOPT_WRITEFUNCTION,	http_loader::curlWriteFunction);
}

http_loader::~http_loader()
{
    curl_easy_cleanup(m_curl);
}

size_t http_loader::curlWriteFunction( void *ptr, size_t size, size_t nmemb, void *stream )
{
    Glib::RefPtr< Gio::MemoryInputStream >* s = (Glib::RefPtr< Gio::MemoryInputStream >*) stream;
    (*s)->add_data(ptr, size * nmemb);
    return size * nmemb;
}

Glib::RefPtr< Gio::InputStream > http_loader::load_file(const litehtml::tstring& url)
{
    m_url = url;

    Glib::RefPtr< Gio::MemoryInputStream > stream = Gio::MemoryInputStream::create();

    if(m_curl)
    {
        curl_easy_setopt(m_curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(m_curl, CURLOPT_WRITEDATA, &stream);
        curl_easy_perform(m_curl);
        char* new_url = NULL;
        if(curl_easy_getinfo(m_curl, CURLINFO_EFFECTIVE_URL, &new_url) == CURLE_OK)
        {
            if(new_url)
            {
                m_url = new_url;
            }
        }
    }

    return stream;
}
