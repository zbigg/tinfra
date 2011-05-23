#include <iostream>
#include <string>

typedef std::string inentifier;

struct fact {
	identifier  target;
	identifier  relation;
	std::string value;
	long        stamp;
	std::string author;
};

typedef std::vector<fact> fact_list;

void fact_create(fact const& nf)
{
	fact_list current = fact_read(nf.target, READ_EMPTY_IF_NOT_EXIST);
	{
		fact nff(nf);
		nff.stamp = timestamp_for_now();
		nff.author = options::author::value();
		current.push_back(nff);
	}
	fact_write(current);
}

void fact_ls_by_targets()
{
	
}

void fact_view(identifier const&)
{
}

namespace {
	expose_operation fc("create(fact)", make_operation(fact_create));
	expose_operation fc("ls", make_operator(fact_ls_by_targets));
	expose_operation fs("view(target)",  make_operation(fact_view));
}

int main()
{
	
}
