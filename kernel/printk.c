#include <inc/common.h>
#include <inc/stdarg.h>


char* convert(unsigned int, int);
int puts(char* s);

/* implement this function to support printk */
int vfprintf(int (*printer)(char), const char *ctl, va_list arg) {
	int count = 0;
	for(; *ctl != '\0'; ctl ++) {
		int32_t i;
		char* s;
		if (*ctl != '%') {
            count += printer(*ctl);
		}
		else switch(*(++ctl)) {
			case 'd':
				i = va_arg(arg, int);
				if(i < 0) {
					i = -i;
					printer('-');
                    count++;
				}
				count += puts(convert(i, 10));
				break;
			case 'x':
				i = va_arg(arg, unsigned int);
				count += puts(convert(i, 16));
				break;
			case 'c':
				i = va_arg(arg, int);
				count += printer(i);
				break; 
			case 's':
				s = va_arg(arg, char *);       //Fetch string
				count += puts(s);
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
	return count;
}

extern int serial_printc(char);

int printk(const char *ctl, ...) {
	va_list arg;
	va_start(arg, ctl);
	vfprintf(serial_printc, ctl, arg);
	va_end(arg);
	return 0;
}

char *convert(unsigned int num, int base) { 
    static char Representation[] = "0123456789ABCDEF";
    static char buffer[50]; 
    char *ptr; 

    ptr = &buffer[49]; 
    *ptr = '\0'; 
    do {
        *--ptr = Representation[num % base]; 
        num /= base; 
    } while(num != 0); 

    return(ptr); 
}

int puts(char* s) {
    int count = 0;
	while(*s != '\0') {
        count += serial_printc(*s);
		s++;
	}
    return count;
}
