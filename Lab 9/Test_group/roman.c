#include <stdio.h>
#include <string.h>
#include <assert.h>

int romanToArabic(char *roman);

void test(void);

int main(int argc, char **argv)
{
	test();
	if(argc==2){
		printf("The roman numeral %s is equal to %d\n",\
		argv[1], romanToArabic(argv[1]));
	}
	else{
		printf("ERROR: Incorrect usage, try e.g. %s XXI\n", argv[0]);
	}
	return 0;
}

int romanToArabic(char *roman)
{
	int i, sum=0;
	
	for(i=strlen(roman)-1; i>=0; i--){
		switch (roman[i]){
			case 'I':
				if(roman[i+1]=='V'||roman[i+1]=='X'){
					sum-=1;
				}
				else{
					sum+=1;
				}
				break;
			case 'V':
				sum+=5;
				break;
			case 'X':
				if(roman[i+1]=='L'||roman[i+1]=='C'){
					sum-=10;
				}
				else{
					sum+=10;
				}
				break;
			case 'L':
				sum+=50;
				break;
			case 'C':
				if(roman[i+1]=='D'||roman[i+1]=='M'){
					sum-=100;
				}
				else{
					sum+=100;
				}
				break;
			case 'D':
				sum+=500;
				break;
			case 'M':
				sum+=1000;
				break;
			default:
				printf("ERROR: It's not a roman numeral!\
				\nPlease input roman numerals e.g. XXI\n");
				assert(0);
		}
	}
	return sum;
}

void test(void)
{
	assert(romanToArabic("MCMXCIX")==1999);
	assert(romanToArabic("MCMLXVII")==1967);
	assert(romanToArabic("MCDXCI")==1491);
}