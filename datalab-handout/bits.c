/* 
 * CS:APP Data Lab 
 * 
 * <kdwon>
 * 
 * bits.c - Source file with your solutions to the Lab.
 *          This is the file you will hand in to your instructor.
 *
 * WARNING: Do not include the <stdio.h> header; it confuses the dlc
 * compiler. You can still use printf for debugging without including
 * <stdio.h>, although you might get a compiler warning. In general,
 * it's not good practice to ignore compiler warnings, but in this
 * case it's OK.  
 */

#if 0
/*
 * Instructions to Students:
 *
 * STEP 1: Read the following instructions carefully.
 */

You will provide your solution to the Data Lab by
editing the collection of functions in this source file.

INTEGER CODING RULES:
 
  Replace the "return" statement in each function with one
  or more lines of C code that implements the function. Your code 
  must conform to the following style:
 
  int Funct(arg1, arg2, ...) {
      /* brief description of how your implementation works */
      int var1 = Expr1;
      ...
      int varM = ExprM;

      varJ = ExprJ;
      ...
      varN = ExprN;
      return ExprR;
  }

  Each "Expr" is an expression using ONLY the following:
  1. Integer constants 0 through 255 (0xFF), inclusive. You are
      not allowed to use big constants such as 0xffffffff.
  2. Function arguments and local variables (no global variables).
  3. Unary integer operations ! ~
  4. Binary integer operations & ^ | + << >>
    
  Some of the problems restrict the set of allowed operators even further.
  Each "Expr" may consist of multiple operators. You are not restricted to
  one operator per line.

  You are expressly forbidden to:
  1. Use any control constructs such as if, do, while, for, switch, etc.
  2. Define or use any macros.
  3. Define any additional functions in this file.
  4. Call any functions.
  5. Use any other operations, such as &&, ||, -, or ?:
  6. Use any form of casting.
  7. Use any data type other than int.  This implies that you
     cannot use arrays, structs, or unions.

 
  You may assume that your machine:
  1. Uses 2s complement, 32-bit representations of integers.
  2. Performs right shifts arithmetically.
  3. Has unpredictable behavior when shifting an integer by more
     than the word size.

EXAMPLES OF ACCEPTABLE CODING STYLE:
  /*
   * pow2plus1 - returns 2^x + 1, where 0 <= x <= 31
   */
  int pow2plus1(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     return (1 << x) + 1;
  }

  /*
   * pow2plus4 - returns 2^x + 4, where 0 <= x <= 31
   */
  int pow2plus4(int x) {
     /* exploit ability of shifts to compute powers of 2 */
     int result = (1 << x);
     result += 4;
     return result;
  }

FLOATING POINT CODING RULES

For the problems that require you to implent floating-point operations,
the coding rules are less strict.  You are allowed to use looping and
conditional control.  You are allowed to use both ints and unsigneds.
You can use arbitrary integer and unsigned constants.

You are expressly forbidden to:
  1. Define or use any macros.
  2. Define any additional functions in this file.
  3. Call any functions.
  4. Use any form of casting.
  5. Use any data type other than int or unsigned.  This means that you
     cannot use arrays, structs, or unions.
  6. Use any floating point data types, operations, or constants.


NOTES:
  1. Use the dlc (data lab checker) compiler (described in the handout) to 
     check the legality of your solutions.
  2. Each function has a maximum number of operators (! ~ & ^ | + << >>)
     that you are allowed to use for your implementation of the function. 
     The max operator count is checked by dlc. Note that '=' is not 
     counted; you may use as many of these as you want without penalty.
  3. Use the btest test harness to check your functions for correctness.
  4. Use the BDD checker to formally verify your functions
  5. The maximum number of ops for each function is given in the
     header comment for each function. If there are any inconsistencies 
     between the maximum ops in the writeup and in this file, consider
     this file the authoritative source.

/*
 * STEP 2: Modify the following functions according the coding rules.
 * 
 *   IMPORTANT. TO AVOID GRADING SURPRISES:
 *   1. Use the dlc compiler to check that your solutions conform
 *      to the coding rules.
 *   2. Use the BDD checker to formally verify that your solutions produce 
 *      the correct answers.
 */


#endif
/* 
 * bitAnd - x&y using only ~ and | 
 *   Example: bitAnd(6, 5) = 4
 *   Legal ops: ~ |
 *   Max ops: 8
 *   Rating: 1
 */
int bitAnd(int x, int y) {
  //by the law of Demorgan
  return ~(~(x)|~(y)); 
}
/* 
 * getByte - Extract byte n from word x
 *   Bytes numbered from 0 (LSB) to 3 (MSB)
 *   Examples: getByte(0x12345678,1) = 0x56
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 6
 *   Rating: 2
 */
int getByte(int x, int n) {
  int temp;
  temp =  x >> (n << 3);//n << 3 is same as n*8, 8비트 단위로 앞쪽 비트로 값을 가져옴
  return temp & 0xFF;//AND연산을 취하면 마지막 바이트를 얻을 수 있다.
}
/* 
 * logicalShift - shift x to the right by n, using a logical shift
 *   Can assume that 0 <= n <= 31
 *   Examples: logicalShift(0x87654321,4) = 0x08765432
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 20
 *   Rating: 3 
 */
int logicalShift(int x, int n) {
  int msb;
  int oper;
  int shift;

  msb = (x >> 31) << 31;//sign bit
  oper = x >> n;//Arithmetic shift
  shift = (!!n << 31) >> 31;//0일 경우에는 0000..0, 나머지는 1111...1
  msb = msb >> ((n + (~1 +1)) & shift);//n-1 만큼 부호 비트만 오른쪽으로 산술 시프트한 결과, shift를 AND 연산하여 0 예외처리

  return oper + ((~msb + 1) & shift );//n-1 만큼 부호 비트만 오른쪽으로 산술 시프트한 결과를 빼주지만 0 예외처리

}
/*
 * bitCount - returns count of number of 1's in word
 *   Examples: bitCount(5) = 2, bitCount(7) = 3
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 40
 *   Rating: 4
 */
int bitCount(int x) {
  // 첫번째 oper은 1과 0이 번갈아가며 나타나는데, 이떄 a와 b를 더하면 인접한 비트간의 합을 2bit단위로 저장할 수 있다.
  //이를 그 다음에는 11과 00이 번갈아가며 나타나는 oper를 사용하면 인접한 2bit 단위간의 합을 구할 수 있다.
  //1111과 0000이 번갈아가며 나타나는 oper를 다음으로 사용한다. 8비트 단위는 그에 해당하는 비트에 저장되어 있는 1의 수를 가진다.
  //8비트 단위로 1의 갯수가 저장되어 있으므로 이를 0xff를 각 단위에 대해 shift하면서 bitwise and 연산을 취하고 더해주면,
  //총 1의 갯수를 구할 수 있다.
  int oper;
  int a;
  int b;
  int sum;

  oper = (0x55 << 8) + 0x55;
  oper = (oper << 16) + oper;
  a = x & oper;
  b = (x >> 1) & oper;
  sum = a + b; 

  oper = (0x33 << 8) + 0x33;
  oper = (oper << 16) + oper;
  a = sum & oper;
  b = (sum >> 2) & oper;
  sum = a + b;
 

  oper = (0x0f << 8) + 0x0f;
  oper = (oper << 16) + oper;
  a = sum & oper;
  b = (sum >> 4) & oper; 
  sum = a + b;

  return (sum & 0xff) + ((sum >> 8) & 0xff) + ((sum >> 16) & 0xff) + ((sum >> 24) & 0xff);
  
}
/* 
 * bang - Compute !x without using !
 *   Examples: bang(3) = 0, bang(0) = 1
 *   Legal ops: ~ & ^ | + << >>
 *   Max ops: 12
 *   Rating: 4 
 */
int bang(int x) {
  //last bit of temp contains result of bitwise OR operation of all bits.
  //So, last bit of temp with bitwise NOT would be answer.
  int temp;
  temp = x;
  temp = (temp >> 16) | temp;
  temp = (temp >> 8) | temp;
  temp = (temp >> 4) | temp;
  temp = (temp >> 2) | temp;
  temp = (temp >> 1) | temp;
  return 0x01 & ~temp;
}
/* 
 * tmin - return minimum two's complement integer 
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 4
 *   Rating: 1
 */
int tmin(void) {
  //by definition of 2's complement
  return (0x01 << 31);
}
/* 
 * fitsBits - return 1 if x can be represented as an 
 *  n-bit, two's complement integer.
 *   1 <= n <= 32
 *   Examples: fitsBits(5,3) = 0, fitsBits(-4,3) = 1
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int fitsBits(int x, int n) {
  // x can be represented as an n bit if 
  int msb;
  int temp;
  int shift;

  shift = n + (~1 + 1);
  msb = (x >> 31);
  temp = (x >> shift);

  return !(msb ^ temp);

}
/*
 * divpwr2 - Compute x/(2^n), for 0 <= n <= 30
 *  Round toward zero
 *   Examples: divpwr2(15,1) = 7, divpwr2(-33,4) = -2
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 15
 *   Rating: 2
 */
int divpwr2(int x, int n) {
  int ind;
  int msb;

  ind = x << (32 + (~n +1));
  msb = (x >> 31) ;
  msb = !(!msb); //MSB가 1이면 1, 0이면 0이다. 
  ind = !(!ind); //x에 2^n 비트 아래에 수가 없으면 0, 있으면 1

  return (x >> n) +((!!n) & ((msb) & (ind))); // n이 0일때 예외처리를 !!n을 &연산 시켜주며 해결한다.
}
/* 
 * negate - return -x 
 *   Example: negate(1) = -1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 5
 *   Rating: 2
 */
int negate(int x) {
  //by definition of 2's complement
  return (~x + 1);
}
/* 
 * isPositive - return 1 if x > 0, return 0 otherwise 
 *   Example: isPositive(-1) = 0.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 8
 *   Rating: 3
 */
int isPositive(int x) {
  //양수 처리이므로, sign bit가 0이더라고 x가 0이 아닌경우에만 1이 되도록한다.
  return (!(x >> 31) ^ !(x));
}
/* 
 * isLessOrEqual - if x <= y  then return 1, else return 0 
 *   Example: isLessOrEqual(4,5) = 1.
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 24
 *   Rating: 3
 */
int isLessOrEqual(int x, int y) {
  // x와 y의 부호가 다른 경우, 같은 경우 로 나누어서 생각한다. 각 경우에 대한 결과값을 모두 OR 연산하여 최종 결과값을 얻는다.
  int sub;
  int tmin;
  int shift_x;
  int shift_y;
  int diff;
  int a0;
  int a1;
  int a2;
  int a3;

  sub = (y + (~x + 1));
  tmin = 0x01 << 31;
  shift_x = x >> 31;
  shift_y = y >> 31;
  diff = !shift_x ^ !shift_y; 

  a0 = !(sub >> 31);
  a1 = (!shift_y & shift_x); 
  a2 = (!shift_y | shift_x); 
  a3 = (!(x^tmin));

  return a3 | ( ((diff)&(a1 | a2)) | ((!diff)&(a0)) );
}
/*
 * ilog2 - return floor(log base 2 of x), where x > 0
 *   Example: ilog2(16) = 4
 *   Legal ops: ! ~ & ^ | + << >>
 *   Max ops: 90
 *   Rating: 4
 */
int ilog2(int x) {
  //MSB아래의 bit가 모두 1이도록 만듬. 그 뒤에 bitcount 를 사용하여 1의 갯수를 샌다.
  //16, 8, 4, 2, 1 차례로 shift하여 MSB 아래가 모두 1이 되도록 만들 수 있다. 
  //log floor는 최고 비트의 weight의 지수값이 되는데, 이는 count -1.
  int oper;
  int a;
  int b;
  int sum;
  int count;

  x = (x >> 16) | x;
  x = (x >> 8) | x;
  x = (x >> 4) | x;
  x = (x >> 2) | x;
  x = (x >> 1) | x;

  oper = (0x55 << 8) + 0x55;
  oper = (oper << 16) + oper;
  a = x & oper;
  b = (x >> 1) & oper;
  sum = a + b;

  oper = (0x33 << 8) + 0x33;
  oper = (oper << 16) + oper;
  a = sum & oper;
  b = (sum >> 2) & oper;
  sum = a + b;

  oper = (0x0f << 8) + 0x0f;
  oper = (oper << 16) + oper;
  a = sum & oper;
  b = (sum >> 4) & oper;
  sum = a + b;

  count = (sum & 0xff) + ((sum >> 8) & 0xff) + ((sum >> 16) & 0xff) + ((sum >> 24) & 0xff);

  return (count +(~1 +1));
}
/* 
 * float_neg - Return bit-level equivalent of expression -f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representations of
 *   single-precision floating point values.
 *   When argument is NaN, return argument.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 10
 *   Rating: 2
 */
unsigned float_neg(unsigned uf) {
  //NaN인 경우와 아닌 경우로 나누어 생각.
  //최고 비트에 1을 더해 부호 바꿔줌
  unsigned result;
  if( (((uf << 1) >> 24) == 0xff) && ((uf << 9) != 0x0) ){
    result = uf;
  }
  else{
    result = uf + (1 << 31);
  }
  return result;
}
/* 
 * float_i2f - Return bit-level equivalent of expression (float) x
 *   Result is returned as unsigned int, but
 *   it is to be interpreted as the bit-level representation of a
 *   single-precision floating point values.
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_i2f(int x) {
  //exp, frac, sign 을 모두 따로 구해서 더한다.
  //이때, MSB 아래의 비트가 23개를 넘어가면 rounding을 해줘야하는데, 변수 half (=1000...0)을 경계로
  //round to even 해준다.
  
  unsigned result = 0;
  int abs_x;
  unsigned sign_bit;
  int temp;
  int temp_;
  unsigned count;
  unsigned exp;
  unsigned frac;
  int round;
  int half;

  unsigned a;
  unsigned b;


  if(x == 0x80000000){
    return 0xcf000000;
  }
  else if (x == 0x00){
    return x;
  }
  else{
    abs_x = x;
    sign_bit = 0;
    if(x < 0){
      abs_x =  -x;
      sign_bit = 0x80000000;
    }
    temp = 0x80000000;
    count = -1;
    while(1){
      count = count + 1;
      if(temp & abs_x){
        break;
      }
      temp = temp >> 1;
    }

    a = 8 - count;
    b = 7 - count;
    half = 0x01 << b;
    temp_ = (~temp);

    exp = (158 - count) << 23;
    frac = abs_x & (temp_); 

    if((31 - count) <= 23){
      frac = frac << (-a);
    }
    else{
      frac = (frac >> (a));
      round = (temp_ >> 23) & abs_x;

      if(round == half){
        unsigned round_temp;
        round_temp = abs_x & (0x01 << (a));
        round_temp = round_temp >> (a);
        if(round_temp){
          result = 1;
        }
      }

      else if(round > half){
        result = 1;
      }
    }

    result = result + exp + frac + sign_bit;
  }
  return result;
}
/* 
 * float_twice - Return bit-level equivalent of expression 2*f for
 *   floating point argument f.
 *   Both the argument and result are passed as unsigned int's, but
 *   they are to be interpreted as the bit-level representation of
 *   single-precision floating point values.
 *   When argument is NaN, return argument
 *   Legal ops: Any integer/unsigned operations incl. ||, &&. also if, while
 *   Max ops: 30
 *   Rating: 4
 */
unsigned float_twice(unsigned uf) {
  //차례로 denormalized number, special case, normalized number로 case 분류
  //special case는 그대로 return하고, normalize는 exp에 1 더해줌
  //denormalized는 sign bit 제외하고 왼쪽으로 1 shift
  unsigned exp_uf;
  unsigned sign_bit;
  unsigned temp;
  unsigned result;

  exp_uf = uf & (0xff << 23);
  exp_uf = exp_uf >> 23; 

  if( !exp_uf ){
    sign_bit = uf & 0x80000000;
    temp = uf & ~sign_bit;
    temp = temp << 1;
    result = temp + sign_bit;
  }
  else if( !(exp_uf - 0xff) ){
    result = uf;
  }
  else{
    result = uf + (0x01 << 23);
  }
  return result;
}
