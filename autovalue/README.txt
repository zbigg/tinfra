autovalue playground

By autovalue i mean "active" value objects
which contain either constant or expression for
creation of value from other AVs.
AV is like spreadsheet cell, it is either formula
or constant.

Usage:
av<int> x(2);
av<int> y = x+2;
assert(y.get() == 4)
x = 4;
assert(y.get() == 6)

Status:
	basic scheme works
	flaws:
		referencing is broken
		"infra" cost is awful
		several huge objects(shared_ptr,std::functionn)
		  used to track one dependecy
