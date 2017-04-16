#include <inc/common.h>
#include <inc/stdarg.h>


char* convert(unsigned int, int);
void puts(char* s);

/* implement this function to support printk */
void vfprintf(void (*printer)(char), const char *ctl, va_list arg) {
	for(; *ctl != '\0'; ctl ++) {
		int32_t i;
		char* s;
		if (*ctl != '%') {
			printer(*ctl);
		}
		else switch(*(++ctl)) {
			case 'd':
				i = va_arg(arg, int);
				if(i < 0) {
					i = -i;
					printer('-');
				}
				puts(convert(i, 10));
				break;
			case 'x':
				i = va_arg(arg, unsigned int);
				puts(convert(i, 16));
				break;
			case 'c':
				i = va_arg(arg, int);   
				printer(i);
				break; 
			case 's':
				s = va_arg(arg, char *);       //Fetch string
				puts(s);
				break; 
			default :
				break;
		}
	}
	va_end(arg);
	/*
	const char *str = __FUNCTION__;
	for(;*str != '\0'; str ++) printer(*str);

	str = "() is not implemented!\n";
	for(;*str != '\0'; str ++) printer(*str);
	*/
}

extern void serial_printc(char);

void printk(const char *ctl, ...) {
	va_list arg;
	va_start(arg, ctl);
	vfprintf(serial_printc, ctl, arg);
	va_end(arg);
}

char *convert(unsigned int num, int base) { 
    static char Representation[] = "0123456789ABCDEF";
    static char buffer[50]; 
    char *ptr; 

    ptr = &buffer[49]; 
    *ptr = '\0'; 
    do{ 
        *--ptr = Representation[num % base]; 
        num /= base; 
    } while(num != 0); 

    return(ptr); 
}

void puts(char* s) {
	while(*s != '\0') {
		serial_printc(*s);
		s++;
	}
}
