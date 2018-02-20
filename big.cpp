#include <mem.h>
#include <stdlib.h>
#include <ctype.h>

class big
{
public:
    static const int SIZE = 72;
    static const int BUFSIZE = 4*SIZE/3 + 2;

    // constructor
    big() {}
    big(int);
    big(const char*) ;

    // comparing functions
    int _cmp(const big&) const;
    int  cmp(const big&) const;
    int  cmp() const { return nonzero() ? 1 - sign - sign: 0;}
    bool operator>(const  big& other) const {return cmp(other) > 0;}
    bool operator<(const  big& other) const {return cmp(other) < 0;}
    bool operator>=(const big& other) const {return cmp(other) >= 0;}
    bool operator<=(const big& other) const {return cmp(other) <= 0;}
    bool operator==(const big& other) const {return cmp(other) == 0;}
    bool operator!=(const big& other) const {return cmp(other) != 0;}
    bool nonzero() const {return size!=1 || digit[0]!=0;}
    bool iszero() const  {return !nonzero();}
    bool ispos() const  {return !sign;}

    // in-place member functions
    big& set_zero();
    big& fix_size(int);
    big& fix_carry(int);
    big& fix_borrow(int);
    big& half();
    big& negate() {sign ^= 1; return *this;}
    big& _add1(int i=0) {return (++digit[i]<10) ? *this : fix_carry(i);}
    big& _sub1(int i=0) {return (--digit[i]>=0) ? *this : fix_borrow(i);}
    big& _add(const big& other);
    big& _sub(const big& other);
    big& operator<<=(int n);
    big& operator>>=(int n);

    // "safe" functions
    big operator<<(int n) const {return big(*this) <<= n;}
    big operator>>(int n) const {return big(*this) >>= n;}
    big operator-() const {return big(*this).negate();}
    big operator+(const big&) const ;
    big operator-(const big& other) const {return operator+(-other);}
    big operator*(const big&) const ;
    big operator/(const big& den) const ;
    big operator%(const big& den) const ;
    big pow(int) const;
    big sqr() const;
    big sqrt() const ;

    int digits() const {return size;}
    int operator[](int i) const {return digit[i];}
    int toint() const;
    char* tostr(char* s) const;

private:
    char digit[SIZE];           // digit[0] = ones, digit[1] = tens
    int sign;                   // negative number ? 1=Yes,0=No
    int size ;                  // number of digits
};

big::big(int n)
{
    set_zero() ;
    if (n) {
        if (n<0) { n = -n; sign = 1; }
        int i=0;
        do {
            digit[i++] = n % 10;
        } while (n /= 10);
        size = i;
    }
}

big::big(const char *s)
{
    set_zero();
    if (*s == '-') sign = 1;                // get sign
    if (sign || *s == '+') ++s;             // go pass sign
    while (*s == '0') ++s;                  // go pass leading 0's

    // get total digits and make sure number is valid
    int i = 0;                              // total digits
    for(const char *p = s; *p ; ++p) {      // go to end of string
        if (*p == ',') continue;            // ignore commas anywhere
        if (!isdigit(*p)) exit(1);          // bad digits
        ++i;                                // 1 more valid digit
    }

    // build number, start from most significant digit
    if (i)                                  // "return" zero if no digits
        for(size = i ; i--; s++)
            if (*s != ',') digit[i] = *s - '0';
}

// Compare absolute value of *this and other. With sign ignored,
// returns <0 if *this<other,
//         =0 if *this==other
//         >0 if *this>other
int big::_cmp (const big & other) const
{
    if (size != other.size) return size - other.size ;
    int i = size;
    while(--i && digit[i]==other.digit[i]) ;
    return digit[i] - other.digit[i];
}

// Compare value of *this and other
int big::cmp(const big& other) const
{
    int m = 1 - sign - sign;                // multiplier
    if (sign == other.sign) return _cmp(other) * m;
    return (nonzero() || other.nonzero()) ? m : 0;
}

big& big::set_zero()
{
    sign=0;
    size=1 ;
    memset(digit, 0, sizeof(digit));
    return *this;
}

// assumed i >= actual digits > 0
big& big::fix_size(int i)
{
    while (--i && digit[i]==0) ;            // find "top" digit idx
    size = i + 1;
    return *this;
}

big& big::operator<<=(int n)
{
    if (n==0 || iszero()) return *this;     // nothing to shift
    if ((size += n) > SIZE) exit(1);        // left shift overflow !
    memmove(digit + n, digit, (size-n) * sizeof(digit[0]));
    memset(digit, 0, n);                    // clear "bottom" digits
    return *this;
}

big& big::operator>>=(int n)
{
    if (n==0 || iszero()) return *this;     // nothing to shift
    if ((size -= n) < 1) return set_zero(); // right shifted every bits !
    memmove(digit, digit + n, size * sizeof(digit[0]));
    memset(digit+size, 0, n);               // clear "top" digits
    return *this;
}

// handle positive and negative numbers
// actual calculations done by _add() and _sub()
big big::operator+(const big& other) const
{
    if (other.iszero())     return big(*this);
    if (sign == other.sign) return big(*this)._add(other);
    if (_cmp(other) >= 0)   return big(*this)._sub(other);
    return big(other)._sub(*this);          // switch side
}

big big::operator*(const big & other) const
{
    big product = 0;
    int i=0, j=0, k=0, mul=0, total=0;

    for(; i < other.size ; i++)
        if ((mul = other.digit[i]) != 0)
            for(j=0, k=i ; j < size ; j++) {
                total = product.digit[k] + mul * digit[j];
                product.digit[k++] = total % 10;
                product.digit[k]  += total / 10;
            }
    product.sign = sign ^ other.sign;
    return product.fix_size(i+j) ;
}

big big::operator/(const big& den) const
{
    if (den.iszero()) exit(1) ;             // divide by 0
    int imax = size-den.size, i=imax;       // quotient max digit index
    if (imax < 0) return big(0);            // no quotient!

    big quot=0, remain=*this;
    if (i>0) {                              // need den shifting
        big d = den;                        // work with a copy
        for(d<<=i ;; d>>=1) {
            for(; remain._cmp(d) >= 0; remain._sub(d)) ++quot.digit[i] ;
            if (--i == 0) break;
        }
    }
    for(; remain._cmp(den) >= 0; remain._sub(den)) ++quot.digit[i] ;
    quot.fix_size(imax + 1);
    quot.sign = remain.sign ^ den.sign;
    if (quot.ispos() || remain.iszero()) return quot;
    return quot._add1();                    // special case
}

big big::operator%(const big& den) const
{
    if (den.iszero()) exit(1) ;             // divide by 0
    int imax = size-den.size, i=imax;       // quotient max digit index
    if (imax < 0) return *this;             // nothing to do

    big remain=*this;
    if (i>0) {                              // need den shifting
        big d = den;                        // work with a copy
        for(d<<=i; ;d>>=1) {
            for(; remain._cmp(d) >= 0; remain._sub(d)) ;
            if (--i == 0) break;
        }
    }
    for(; remain._cmp(den) >= 0; remain._sub(den)) ;
    if (sign == den.sign || remain.iszero()) return remain;
    return big(den)._sub(remain);           // special case
}

big big::pow(int n) const
{
    if (n==1) return *this;
    if (n &1) return pow(n/2).sqr() * (*this);
    return pow(n/2).sqr();
}

big big::sqr() const                        // fast way to make square
{
    big square = 0;                         // square is positive
    int i, j, k, sum, total=0;

    for(i=0; i < size+size-1; i++) {        // build all digits of square
        for(sum=0, j=(i<size)?i:size-1, k=i-j; j > k ; j--, k++)
            sum += digit[j] * digit[k];
        total += (i % 2) ? 2 * sum          // use symmetry
                         : 2 * sum + digit[j] * digit[j];
        square.digit[i] = total % 10;
        total /= 10;                        // carry to the next digit
    }
    if (total) square.digit[i++] = total;   // one more digit
    square.size = i;
    return square;
}

big big::sqrt() const                       // find root by Newton's method
{
    big x0, x1 ,x2=0;

    if (iszero()) return *this ;            // nothing to do
    if (sign) exit(1) ;                     // square root of negative
    x2.size = size/2 ;                      // initial guess
    x2.digit[x2.size++] = 1 ;
    do {
        x0 = x2 ;
        x1 = *this / x0 ;
        x2._add(x1).half() ;                // new estimate done in-place
    } while(x2._cmp(x1) && x2._cmp(x0));    // quit if converged
    return x2 ;                             // note: x2 * x2 <= *this
}

big& big::half()                            // halving in place
{
    for(int i=0; i<size; i++) {
        digit[i] /= 2 ;
        if (digit[i+1] & 1)                 // look ahead for carry
            digit[i] += 5 ;                 // do carry
    }
    return fix_size(size) ;                 // possibly lost 1 digit
}

big& big::fix_carry(int i)
{
    digit[i++] -= 10;                       // digit[i] between 10 to 19
    while (digit[i]==9) digit[i++]=0;       // carry forward
    ++digit[i++];
    if (i > size) size = i;                 // update digits
    return *this;
}

big& big::fix_borrow(int i)
{
    digit[i++] += 10;                       // digit[i] between -1 to -10
    while (digit[i]==0) digit[i++]=9;       // borrow forward
    if (--digit[i] != 0) return *this;      // top digit still here
    if (size == i+1) size = i;              // top digit gone
    return *this;
}

// abs(*this) += abs(other), keeping *this sign

big& big::_add(const big & other)
{
    int i=0;
    do {
        digit[i] += other.digit[i];
        if (digit[i] < 10) {                // no carry
            ++i;
        } else {                            // do carry
            digit[i++] -= 10;               // update digit
            ++digit[i];                     // borrow
        }
    } while (i < other.size);

    if (digit[i] < 10) {                    // no extra carry
        return (i < size) ? *this : fix_size(i+1);
    } else
        return fix_carry(i);
}

// assumed *this >= other
// abs(*this) -= abs(other), keeping *this sign
big& big::_sub(const big & other)
{
    int i=0;
    do {
        digit[i] -= other.digit[i];
        if (digit[i] >= 0) {                // no borrow
            ++i;
        } else {                            // do borrow
            digit[i++] += 10;               // update digit
            --digit[i];                     // borrow
        }
    } while (i < other.size);

    return (digit[i]<0) ? fix_borrow(i) : fix_size(size);
}

int big::toint() const                      // avoid implicit conversion
{
    int i = size - 1;
    int x = digit[i];                       // most significant digit
    while (i--)
        x = 10 * x + digit[i];              // remaining digits
    return ispos() ? x : -x;                // "add" sign to int
}

char* big::tostr(char* s) const
{
    memset(s, ',', BUFSIZE);
    int i=0, j=size-1;
    int comma = j%3 + 1;
    if (sign && nonzero()) s[i++] = '-';    // do not show -0
    while(j) {
        s[i++] = digit[j--] + '0';
        if (i == comma) i++, comma+=4;      // skip commas
    }
    s[i] = digit[0] + '0';
    s[i+1] = 0;                             // close string
    return s;
}
