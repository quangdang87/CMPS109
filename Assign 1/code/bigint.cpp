// Quang Dang
// qvdang
// $Id: bigint.cpp,v 1.2 2019-04-11 09:24:20-07 - - $

#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
using namespace std;

#include "bigint.h"
#include "debug.h"
#include "relops.h"
bigint::bigint (long that ) : uvalue( that ), is_negative( that > 0 ){}

bigint::bigint (const ubigint& uvalue_, bool is_negative_):
                uvalue(uvalue_), is_negative(is_negative_) {
}

bigint::bigint (const string& that) {
   is_negative = that.size() > 0 and that[0] == '_';
   uvalue = ubigint (that.substr (is_negative ? 1 : 0));
}

bigint bigint::operator+ () const {
   return *this;
}

bigint bigint::operator- () const {
   return {uvalue, not is_negative};
}

bigint bigint::operator+ (const bigint& that) const {
    bigint result;
    if( is_negative == that.is_negative ) {
        result.is_negative = is_negative;
        result.uvalue = uvalue + that.uvalue;    
    }
    else {
        if( uvalue < that.uvalue ) {
            result.is_negative = that.is_negative;
            result.uvalue = that.uvalue - uvalue;
        }
        else if( uvalue == that.uvalue ) {
            result.is_negative = false;
            result.uvalue = 0;
        }
        else if ( that.uvalue < uvalue ) {
            result.is_negative = is_negative;
            result.uvalue = uvalue - that.uvalue;
        }
    }
    return result;
}

bigint bigint::operator- (const bigint& that) const {
    bigint result;
    if( is_negative == that.is_negative ) {
        if( uvalue < that.uvalue ) {
            result.is_negative = false;
            result.uvalue = that.uvalue - uvalue;
        }
        else if( uvalue == that.uvalue ) {
            result.is_negative = false;
            result.uvalue = 0;
        }
        else if( that.uvalue < uvalue ) {
            result.is_negative = true;
            result.uvalue = uvalue - that.uvalue;
        }
    }
    else if( is_negative != that.is_negative ) {
        result.uvalue = uvalue + that.uvalue;
        if( !that.is_negative ) {
            result.is_negative = true;
        } 
        else {
            result.is_negative = false;
        }
    }
    return result;
}

bigint bigint::operator* (const bigint& that) const {
    bigint result;
    result.uvalue = uvalue * that.uvalue;
    if( is_negative == that.is_negative ) {
        result.is_negative = false;
    }
    else {
        result.is_negative = true;    
    }
    return result;
}

bigint bigint::operator/ (const bigint& that) const {
   bigint result;
   result.uvalue = uvalue / that.uvalue;
   if( is_negative != that.is_negative ) {
       result.is_negative = true;
   }
   else {
       result.is_negative = false;
   }
   return result;
}

bigint bigint::operator% (const bigint& that) const {
   bigint result;
   result.uvalue = uvalue % that.uvalue;
   result.is_negative = is_negative;
   return result;
}

bool bigint::operator== (const bigint& that) const {
   return is_negative == that.is_negative and uvalue == that.uvalue;
}

bool bigint::operator< (const bigint& that) const {
   if (is_negative != that.is_negative) return is_negative;
   return is_negative ? uvalue > that.uvalue
                      : uvalue < that.uvalue;
}

ostream& operator<< (ostream& out, const bigint& that) {
    if( that.is_negative) {
        out << "-" << that.uvalue;
    }
    else {
        out << that.uvalue;
    }
   return out;
}

