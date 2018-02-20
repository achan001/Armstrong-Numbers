# Armstrong-Numbers
search for all Armstrong Numbers ***FAST***

For a n-digit number, it is **Armstrong number** if sum of its digits raise to nth power equals itself  

Example: 371 = 27 + 343 + 1 = 3^3 + 7^3 + 1^3

Instead of searching all n-digits number for a match, it only search possible digits combinations  
Using above example, after tested 371, it will **NOT** test 137, 173, 317, 713, 731.  

If no arguments given, produce complete solutions (on *my laptop*, time needed = ***355.6 sec***)  
May be useful for testing new computer ? (Below is ***largest*** Armstrong number = ***16.2 sec***)
  
>d:\cpp> arm 39  
n = 39  
1 > 115,132,219,018,763,992,565,095,597,973,971,522,400  
2 > 115,132,219,018,763,992,565,095,597,973,971,522,401  
  
Note: this program ignored trivial case for n=1 --> smallest Armstrong number = 153  
Reference: http://mathworld.wolfram.com/NarcissisticNumber.html  
