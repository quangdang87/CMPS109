// Quang Dang
// qvdang
// $Id: ubigint.cpp,v 1.2 2019-04-11 09:24:20-07 - - $

#include <cctype>
#include <cmath>
#include <cstdlib>
#include <exception>
#include <stack>
#include <stdexcept>
using namespace std;

#include "ubigint.h"
#include "debug.h"
ubigint::ubigint (unsigned long x ) {
    string numbers = to_string( x );
    for ( long unsigned i = 0; i <= numbers.size() - 1; ++i ) {
        ubig_value.push_back( numbers[i] );
    }
}
ubigint::ubigint (const string& that) {
    DEBUGF ('~', "that =\"" << that << "\"");
    for (int i = that.size()-1; i >= 0; i-- ) {
        if( not isdigit( that[i] )){
            throw invalid_argument( "ubigint::ubigint(" + that + ")");
        }
        ubig_value.push_back( that[i] );
    }
}
ubigint ubigint::operator+ ( const ubigint& that ) const {
    ubigint rhs = that;
    ubigint lhs = *this;
    int same_size = lhs.ubig_value.size() - 
                   rhs.ubig_value.size();
    same_size = abs( same_size );
    int operand1, operand2, carry, res;
    ubigint result;
    unsigned char zero = '0';
    if( same_size > 0 ) {
        for( int i = 0; i < same_size; i++ ) {
            if( lhs.ubig_value.size() > rhs.ubig_value.size() ) {
                rhs.ubig_value.push_back( zero );
            }
            else if( lhs.ubig_value.size() < rhs.ubig_value.size() ) {
                lhs.ubig_value.push_back( zero );
            }
        }
    }
    carry = 0;
    for( long unsigned int i = 0; i <= ubig_value.size() - 1; i++ ) {
        operand1 = ubig_value[i] - '0';
        operand2 = that.ubig_value[i] - '0'; 
        if( (operand1 + operand2 + carry) <= 9 ) {
            res = operand1 + operand2 + carry;
            carry = 0;
        }
        else {
            res = (operand1 + operand2 + carry) % 10;
            carry = 1;
        }
        result.ubig_value.push_back( static_cast< char >( res + 48 ) );
    }
    if( carry != 0 ) {
        result.ubig_value.push_back(
              static_cast< char >( carry + 48 ) );
    }
    return result;
}

ubigint ubigint::operator- (const ubigint& that) const {
    if( *this < that ) { 
        throw  domain_error( "ubigint::operator -(a < b)" );
    }
    ubigint lhs, rhs, result;
    lhs = *this;
    rhs = that;
    int same_size = ubig_value.size() - that.ubig_value.size();
    same_size = abs( same_size );
    if( lhs.ubig_value.size() > rhs.ubig_value.size() ) {
           rhs.ubig_value.push_back( 
                static_cast< unsigned char >( 48 ) ); 
    }
    else if( lhs.ubig_value.size() < rhs.ubig_value.size() ) {
         lhs.ubig_value.push_back( 
                static_cast< unsigned char >( 48 ) );
    }
    int carry = 0;
    int operand1, operand2;
    int res;
    for( long unsigned i = 0; i <= lhs.ubig_value.size() - 1; i++ ) {
        operand1 = lhs.ubig_value[i] - 48;
        operand2 = rhs.ubig_value[i] - 48;
        if( operand1 < ( operand2 + carry ) ) {
            res = (operand1 + 10) - (operand2 + carry);
            carry = 1;
        }
        else {
            res = operand1 - (operand2 + carry);
            carry = 0;
        }
        result.ubig_value.push_back( static_cast< char >( res + 48 ));
    }
    return result;
}

ubigint ubigint::operator* (const ubigint& that) const {
    ubigint result;
    int carry, oper1, oper2;
    int size = ubig_value.size() + that.ubig_value.size();
    vector< int > res( size, 0 ); 
    for( long unsigned i = 0; i <= that.ubig_value.size() - 1; ++i ) {
        carry = 0;
        for ( long unsigned j = 0; j <= ubig_value.size() - 1; ++j ) {
            oper1 = that.ubig_value[i] - 48;
            oper2 = ubig_value[j] - 48;
            int d = res[i+j] + oper1*oper2 + carry;
            res[i+j] = d%10;
            carry = floor( d/10 );            
        }
        res[i+ ubig_value.size()] = carry;
    }
    int x = res.size() -1;
    while( x >= 0 && res[x] == 0) {
        res.pop_back();
        x -= 1;
    }
    for(long unsigned i = 0; i <= res.size() - 1; ++i) {
        result.ubig_value.push_back( 
            static_cast< char >( res[i] + 48 ) );
    }
    return result;
}

void ubigint::multiply_by_2() {
    int oper, res;
    int carry = 0;
    vector< int > temp;
    for( long unsigned i = 0; i <= ubig_value.size() -1; ++i ) {
        oper = ubig_value[i] - 48;
        res = oper * 2 + carry;
        if( res >= 10 ) {
            carry = res/10;
            res = res%10;
        }
        else {
            carry = 0;
        }
        temp.push_back(res);
        int x = temp.size() - 1;
        while( x >= 0 and temp[x] == 0 ) {
            temp.pop_back();
        }
    }
    ubig_value.clear();
    for( long unsigned i =0; i <= temp.size() - 1; ++i ) {
        ubig_value.push_back( 
            static_cast< unsigned char >( temp[i] + 48) ); 
    }
    temp.clear();
}

void ubigint::divide_by_2() {
    int carry = 0;
    int res, oper;
    int next_num;
    res = 0;
    vector < int > temp;    
    for( long unsigned i = 0; i <= ubig_value.size() - 1; ++i ) {
        oper = ubig_value[i] - 48;
        if( (i + 1) < ubig_value.size() )  {
            next_num = ubig_value[ i + 1 ] - 48;
            if (next_num % 2 != 0 ) {
                carry = 5;
            }
            else {
                carry = 0;
            }
        }
        else {
            carry = 0;
        }
        res = oper/2 + carry;
        temp.push_back( res );
    }
    int x = temp.size() - 1;
    while( x >= 0 and temp[x] == 0 ) {
        temp.pop_back();
    }
    ubig_value.clear();
    for( long unsigned i = 0; i <= temp.size() - 1; ++i ) {
        ubig_value.push_back( static_cast< unsigned char >( temp[i] ));
    }
    temp.clear();
}

struct quo_rem { ubigint quotient; ubigint remainder; };
quo_rem udivide (const ubigint& dividend, ubigint divisor) {
   // Note: divisor is modified so pass by value (copy).
   ubigint zero {0};
   if (divisor == zero) throw domain_error ("udivide by zero");
   ubigint power_of_2 {1};
   ubigint quotient {0};
   ubigint remainder {dividend}; // left operand, dividend
   while (divisor < remainder) {
      divisor.multiply_by_2();
      power_of_2.multiply_by_2();
   }
   while (power_of_2 > zero) {
      if (divisor <= remainder) {
         remainder = remainder - divisor;
         quotient = quotient + power_of_2;
      }
      divisor.divide_by_2();
      power_of_2.divide_by_2();
   }
   return {.quotient = quotient, .remainder = remainder};
}

ubigint ubigint::operator/ (const ubigint& that) const {
   return udivide (*this, that).quotient;
}

ubigint ubigint::operator% (const ubigint& that) const {
   return udivide (*this, that).remainder;
}

bool ubigint::operator== (const ubigint& that) const {
    if( this == &that ) {
        return true;
    }
    else if( ubig_value.size() != that.ubig_value.size() ){
        return false;
    }
    else {
        int oper1, oper2;
        for( int i = ubig_value.size() -1; i >= 0; --i ) {
            oper1 = ubig_value[i] - 48;
            oper2 = that.ubig_value[i] - 48;
            if (oper1 !=  oper2 ) {
                return false;
            }
        }
        return true;
    }
}

ubigint ubigint::operator= (const ubigint& that ) {
    if( &that == this ) {
        return *this;
    }
    else {
        ubig_value.clear();
        for( long unsigned int i = 0; i < that.ubig_value.size(); i++ ){
            ubig_value.push_back( that.ubig_value[i] );
        }
        return *this;
    }
}

ubigint ubigint::operator= (long x ) {
    long i = abs( x );
    ubig_value.clear();
    ubig_value.push_back( static_cast< char >( i + 48 ) );
    return *this;
}
bool ubigint::operator< (const ubigint& that) const {
    if( ubig_value.size() < that.ubig_value.size() ) {
        return true;
    }
    else if( ubig_value.size() > that.ubig_value.size() ) {
        return false;
    }
    else {
         for( long i = ubig_value.size()- 1; i >= 0; i-- ) {
             if( static_cast<int> ( ubig_value[i] - 48 ) >= 
                       static_cast<int> (that.ubig_value[i]) - 48) {
                 return false;
             }
         }
        return true;
    }
    return false;
}

ostream& operator<< (ostream& out, const ubigint& that) { 
    for( int i = that.ubig_value.size()-1; i >= 0; i-- ) {
         out << that.ubig_value[i];
    }
   return out;
}

