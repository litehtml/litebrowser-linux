#pragma once

typedef std::vector<std::string> string_vector;

class web_history
{
	string_vector				m_items;
	string_vector::size_type	m_current_item;
public:
	web_history();
	virtual ~web_history();

	void url_opened(const std::string& url);
	bool back(std::string& url);
	bool forward(std::string& url);
};