#include <stdlib.h>
#include <stdio.h>
#include "big.cpp"

#define DIGIT(n)    counter[2*(n)+1]    // digit counter
#define DCOPY(n)    counter[2*(n)]      // digit counter copy
#define COPY(n)     DCOPY(n) = DIGIT(n)

// Find n-Digit Armstrong number using digit combinations
// return number of solutions
int armstrong(int n)
{
    if (unsigned(n-2) > 58) return 0;   // outside range [2,60]

    int  i, j;                          // index
    int  found = 0;                     // number of solution found
    int  counter[20] = {0};             // digit counter + its copy
    big  arm = 0;                       // arm value of digit counter
    big  diff[n*8], t[10], *p=diff-1;   // lookup tables
    char soln[big::BUFSIZE];            // holds armstrong number

    // make lookup tables
    t[9] = big(9).pow(n);
    for (i=8; i; i--) {
        t[i] = big(i).pow(n);
        p[i] = t[i+1] - t[i];           // difference table for no carry
        for (j=1; j<n; j++)             // build upto n-1 carries
            p[8*j+i] = p[8*(j-1)+i] - t[i];
    }

    // starting pattern that barely produce n-digit arm value
    int low = 9;                        // smallest digit in counter
    for(i=n ; i ; --i ) {               // fill 1 digit at a time
        if (arm.digits() == n) break;
        while(--low)
            if((arm + t[low] * i).digits() < n) break;
        ++DIGIT(++low);                 // ++low as the smallest digit
        arm._add(t[low]);               // now, arm may reach n-digit
    }
    if (i) low=0, DIGIT(0)=i;           // total counter digits == n
    p += low + !low;                    // next needed table entry

    // Ending pattern can have only so many 9's
    t[0] = big(10).pow(n) / t[9];       // max 9's possible
    int max9 = t[0].toint();            // convert to int
    if (max9 >= n) max9 = n-1;          // Tried n 9's by hand

// For each digit pattern, we are going to match digit distribution of
// arm value against the counter that produce it.  If arm has the same
// digits distribution, we have found a Armstrong Number.
// Digit matching "destroy" the counter, so we work with its copy

    for(;;) {
        COPY(0); COPY(1); COPY(2); COPY(3); COPY(4);
        COPY(5); COPY(6); COPY(7); COPY(8); COPY(9);
        
        for(i=n; DCOPY(arm[--i])--;)    // assumed arm is n-digit number
            if (i == 0) {               // n digits of arm matched
                if (arm.digits() == n)  // Armstrong number found !
                    printf("%d > %s\n", ++found, arm.tostr(soln));
                break;
            }

// get next digit counter pattern
// this data structure has the following points :
// (1) it represent ALL combinations = C(n+9,n) - skipped start/end patterns
// (2) will never miss a valid Armstrong number
// (3) first digits >= second >= third >= ... low >= 0
// (4) Solution should be "fairly" sorted from small to big
//
// the actual sequence go like this:
// say, start at 332, then we have 333 -> 400 -> 410 -> 411 -> 420 ...

        switch (low) {
            case 0:                     // special case
                --DIGIT(0);             // e.g. 400 -> 410
                ++DIGIT(1);
                low = !DIGIT(0) ;       // no zeroes -> low = 1
                arm._add1();
                break;
            case 8:
                if (DIGIT(9) == max9) return found;
                /* fall thru */
            default:                    // digits 1 thru 8
                DIGIT(0) = --DIGIT(low);
                if (DIGIT(0)) {         // do carry
                    p += 8 * DIGIT(0);  // go *down* the diff table
                    ++DIGIT(low + 1);   // e.g. 333 -> 400
                    low = DIGIT(low) = 0;
                    p->ispos() ? arm._add(*p) : arm._sub(*p);
                    p = diff;           // reset to top of table
                } else {                // no carry
                    ++DIGIT(++low);     // e.g. 332 -> 333
                    arm._add(*p++);
                }
        }
    }
}

int main(int argc, char* argv[])
{
  int n=2, end=60, solns=0;
  if (argc > 1) end = n = atoi(argv[1]);
  if (argc > 2) end = atoi(argv[2]);

  while(n <= end) {
    printf("n = %d\n", n);
    solns += armstrong(n++);
  }
  return solns;
}
