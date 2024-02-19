#ifndef LITEBROWSER_HTML_DUMPER_H
#define LITEBROWSER_HTML_DUMPER_H

#include <litehtml.h>
#include <fstream>

#define BUFF_SIZE    (10 * 1024)

class html_dumper : public litehtml::dumper
{
	std::ofstream m_cout;
	int indent;
	std::list<std::tuple<int, std::string>> m_node_text;
private:
	void print_indent(int size)
	{
		m_cout << litehtml::string(size, '\t');
	}

public:
	explicit html_dumper(const litehtml::string& file_name) : m_cout(file_name), indent(0)
	{

	}

	void begin_node(const litehtml::string &descr) override
	{
		m_node_text.emplace_back(indent, "#" + descr);
		indent++;
	}

	void end_node() override
	{
		indent--;
	}

	void begin_attrs_group(const litehtml::string &/*descr*/) override
	{
	}

	void end_attrs_group() override
	{
	}

	void add_attr(const litehtml::string &name, const litehtml::string &value) override
	{
		if(name == "display" || name == "float")
		{
			std::get<1>(m_node_text.back()) += " " + name + "[" + value + "]";
		}
	}

	void print()
	{
		for(const auto& data : m_node_text)
		{
			print_indent(std::get<0>(data));
			m_cout << std::get<1>(data) << std::endl;
		}
	}
};

#endif //LITEBROWSER_HTML_DUMPER_H
