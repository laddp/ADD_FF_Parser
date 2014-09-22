#ifndef PML_MONEYPUNCT_H
#define PML_MONEYPUNCT_H
#include <string>
#include <locale>

class pml_moneypunct : public std::moneypunct_byname<char>
{
public:
	pml_moneypunct(const char *name):
		std::moneypunct_byname<char>(name)
	{
		p.field[0] = sign;
		p.field[1] = symbol;
		p.field[2] = none;
		p.field[3] = value;
	}

	virtual std::string do_grouping(void) const { return "\3"; }
	virtual int do_frac_digits(void) const { return 2; }
	virtual string_type do_curr_symbol(void) const { return "$"; }
        virtual char do_decimal_point(void) const { return '.'; }
        virtual char do_thousands_sep(void) const { return ','; }
	virtual struct pattern do_pos_format(void) const { return p; }
	virtual struct pattern do_neg_format(void) const { return p; }

private:
	struct pattern p;
};

#endif // PML_MONEYPUNCT_H
