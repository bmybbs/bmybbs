/*

  Author: Tomi Salo <ttsalo@ssh.fi>

  Copyright (C) 1996 SSH Communications Security Oy, Espoo, Finland
  See COPYING for distribution conditions.

  Implementation of functions snprintf() and vsnprintf()

  */

/*
 * $Id: snprintf.c,v 1.1.1.1 2009-03-04 06:33:27 bmybbs Exp $
 * $Log: snprintf.c,v $
 * Revision 1.1.1.1  2009-03-04 06:33:27  bmybbs
 * bmysrc
 *
 * Revision 1.1.1.1  2002/10/01 09:42:06  clearboy
 * update on 20051031
 * by clearboy 
 * for transfering the source codes from main site to the experimental site 
 * for the first time.
 *
 *
 * Revision 1.1.1.1  2002/10/01 09:42:06  ylsdd
 * 水木底sshbbsd导入
 * 然后慢慢改吧
 *
 * Revision 1.3  2002/08/04 11:39:43  kcn
 * format c
 *
 * Revision 1.2  2002/08/04 11:08:48  kcn
 * format C
 *
 * Revision 1.1.1.1  2002/04/27 05:47:26  kxn
 * no message
 *
 * Revision 1.1  2001/07/04 06:07:12  bbsdev
 * bbs sshd
 *
 * Revision 1.3  1999/02/23 08:30:59  tri
 * 	Downgraded includes to ssh1 style.
 *
 * Revision 1.2  1999/02/23 07:06:17  tri
 *      Fixed compilation errors.
 *
 * Revision 1.1  1999/02/21 19:52:37  ylo
 *      Intermediate commit of ssh1.2.27 stuff.
 *      Main change is sprintf -> snprintf; however, there are also
 *      many other changes.
 *
 * Revision 1.23  1998/11/05 21:14:05  ylo
 *      Changed to ignore return value from sprintf (it returns char *
 *      on some systems, e.g. SunOS 4.1.x).
 *
 * Revision 1.22  1998/10/28 05:19:06  ylo
 *      Changed to compile in the kernel environment if KERNEL is defined.
 *
 * Revision 1.21  1998/10/06 11:32:34  sjl
 *      Fixed previous.
 *
 * Revision 1.20  1998/10/05 10:16:01  sjl
 *      Added (char) cast to format_str_ptr, as suggested by someone
 *      who compiled with IRIX (or Ultrix).
 *
 * Revision 1.19  1998/06/03 00:45:30  ylo
 *      Fixed initialization of prefix and flags.
 *
 * Revision 1.18  1998/03/28  13:18:57  ylo
 *      Changed snprintf to return the length of the resulting string
 *      (not len+1) when the string fits in the buffer.
 *
 * Revision 1.17  1998/01/28 10:14:31  ylo
 *      Major changes in util library.  Several files were renamed,
 *      and many very commonly used functions and types were renamed
 *      (mostly to add ssh prefix).  The event loop and timeout
 *      interface was changed, and split to portable and
 *      machine-dependent parts.  Thousands of calls to
 *      renamed/modified functions were changed accordingly.
 *
 * Revision 1.16  1997/12/28 02:22:25  ylo
 *      Removed unused variable tmp.
 *
 * Revision 1.15  1997/09/18 06:07:41  kivinen
 *      Added check that if we print NULL pointer print (null) just
 *      like normal printf will do.
 *
 * Revision 1.14  1997/06/04 21:54:51  ylo
 *      Added include snprintf.h.
 *      Removed obsolete xmalloc/xfree pair.
 *
 * Revision 1.13  1996/11/03 19:41:29  ttsalo
 *       Kludged & fixed snprintf
 *
 * Revision 1.12  1996/11/03 18:47:56  ttsalo
 *       Changed buffering in snprintf_convert_float
 *
 * Revision 1.11  1996/11/03 17:46:24  ttsalo
 *       Removed references to bsd's float conversion
 *
 * Revision 1.10  1996/11/03 17:44:18  ttsalo
 *       Re-implemented float conversion, uses system's sprintf
 *
 * Revision 1.9  1996/08/21 15:32:47  kivinen
 *      Removed xmalloc.h and fatal.h includes, as they are now in
 *      includes.h.
 *
 * Revision 1.8  1996/08/09 14:44:03  ttsalo
 *      Fixed a problem in non-null-terminated string formatting
 *
 * Revision 1.7  1996/08/09 13:41:59  ttsalo
 *        Don't pad left-justified numbers with zeros
 *
 * Revision 1.6  1996/08/09 12:11:10  ttsalo
 *      More fine-tuning of float-printing
 *
 * Revision 1.5  1996/08/09 11:37:20  ttsalo
 *      Ansi C - conforming snprintf and vsnprintf.
 *
 * Revision 1.4  1996/08/07 10:12:50  ttsalo
 *      Fixed the banner again
 *
 * Revision 1.3  1996/08/07 10:08:24  ttsalo
 *      Fixed the banner
 *
 * Revision 1.2  1996/08/07 09:52:07  ttsalo
 *      Removed diagnostic printf:s
 *
 * Revision 1.1  1996/08/06 19:00:40  ttsalo
 *      Implementation of snprintf and vsnprintf. Everything except
 *      floating point conversions ready.
 *
 * $EndLog$
 */

#include "includes.h"
#include "xmalloc.h"
#include "snprintf.h"

#undef isdigit
#define isdigit(ch) ((ch) >= '0' && (ch) <= '9')

#define MINUS_FLAG 0x1
#define PLUS_FLAG 0x2
#define SPACE_FLAG 0x4
#define HASH_FLAG 0x8
#define CONV_TO_SHORT 0x10
#define IS_LONG_INT 0x20
#define IS_LONG_DOUBLE 0x40
#define X_UPCASE 0x80
#define IS_NEGATIVE 0x100
#define UNSIGNED_DEC 0x200
#define ZERO_PADDING 0x400

#undef sprintf

/* Extract a formatting directive from str. Str must point to a '%'. 
   Returns number of characters used or zero if extraction failed. */

int snprintf_get_directive(const char *str, int *flags, int *width, int *precision, char *format_char, va_list * ap)
{
    int length, value;
    const char *orig_str = str;

    *flags = 0;
    *width = 0;
    *precision = 0;
    *format_char = (char) 0;

    if (*str == '%') {
        /* Get the flags */
        str++;
        while (*str == '-' || *str == '+' || *str == ' ' || *str == '#' || *str == '0') {
            switch (*str) {
            case '-':
                *flags |= MINUS_FLAG;
                break;
            case '+':
                *flags |= PLUS_FLAG;
                break;
            case ' ':
                *flags |= SPACE_FLAG;
                break;
            case '#':
                *flags |= HASH_FLAG;
                break;
            case '0':
                *flags |= ZERO_PADDING;
                break;
            }
            str++;
        }

        /* Don't pad left-justified numbers withs zeros */
        if ((*flags & MINUS_FLAG) && (*flags & ZERO_PADDING))
            *flags &= ~ZERO_PADDING;

        /* Is width field present? */
        if (isdigit(*str)) {
            for (value = 0; *str && isdigit(*str); str++)
                value = 10 * value + *str - '0';
            *width = value;
        } else if (*str == '*') {
            *width = va_arg(*ap, int);

            str++;
        }

        /* Is the precision field present? */
        if (*str == '.') {
            str++;
            if (isdigit(*str)) {
                for (value = 0; *str && isdigit(*str); str++)
                    value = 10 * value + *str - '0';
                *precision = value;
            } else if (*str == '*') {
                *precision = va_arg(*ap, int);

                str++;
            } else
                *precision = 0;
        }

        /* Get the optional type character */
        if (*str == 'h') {
            *flags |= CONV_TO_SHORT;
            str++;
        } else {
            if (*str == 'l') {
                *flags |= IS_LONG_INT;
                str++;
            } else {
                if (*str == 'L') {
                    *flags |= IS_LONG_DOUBLE;
                    str++;
                }
            }
        }

        /* Get and check the formatting character */

        *format_char = *str;
        str++;
        length = str - orig_str;

        switch (*format_char) {
        case 'i':
        case 'd':
        case 'o':
        case 'u':
        case 'x':
        case 'X':
        case 'f':
        case 'e':
        case 'E':
        case 'g':
        case 'G':
        case 'c':
        case 's':
        case 'p':
        case 'n':
            if (*format_char == 'X')
                *flags |= X_UPCASE;
            if (*format_char == 'o')
                *flags |= UNSIGNED_DEC;
            return length;

        default:
            return 0;
        }
    } else {
        return 0;
    }
}

/* Convert a integer from unsigned long int representation 
   to string representation. This will insert prefixes if needed 
   (leading zero for octal and 0x or 0X for hexadecimal) and
   will write at most buf_size characters to buffer.
   tmp_buf is used because we want to get correctly truncated
   results.
   */

int snprintf_convert_ulong(char *buffer, size_t buf_size, int base, char *digits, unsigned long int ulong_val, int flags, int width, int precision)
{
    int tmp_buf_len = 100 + width, len;
    char *tmp_buf, *tmp_buf_ptr, prefix[2];

    tmp_buf = xmalloc(tmp_buf_len);

    prefix[0] = '\0';
    prefix[1] = '\0';

    /* Make tmp_buf_ptr point just past the last char of buffer */
    tmp_buf_ptr = tmp_buf + tmp_buf_len;

    /* Main conversion loop */
    do {
        *--tmp_buf_ptr = digits[ulong_val % base];
        ulong_val /= base;
        precision--;
    }
    while ((ulong_val != 0 || precision > 0) && tmp_buf_ptr > tmp_buf);

    /* Get the prefix */
    if (!(flags & IS_NEGATIVE)) {
        if (base == 16 && (flags & HASH_FLAG))
            if (flags && X_UPCASE) {
                prefix[0] = 'x';
                prefix[1] = '0';
            } else {
                prefix[0] = 'X';
                prefix[1] = '0';
            }

        if (base == 8 && (flags & HASH_FLAG))
            prefix[0] = '0';

        if (base == 10 && !(flags & UNSIGNED_DEC) && (flags & PLUS_FLAG))
            prefix[0] = '+';
        else if (base == 10 && !(flags & UNSIGNED_DEC) && (flags & SPACE_FLAG))
            prefix[0] = ' ';
    } else
        prefix[0] = '-';

    if (prefix[0] != '\0' && tmp_buf_ptr > tmp_buf) {
        *--tmp_buf_ptr = prefix[0];
        if (prefix[1] != '\0' && tmp_buf_ptr > tmp_buf)
            *--tmp_buf_ptr = prefix[1];
    }

    len = (tmp_buf + tmp_buf_len) - tmp_buf_ptr;

    if (len <= buf_size) {
        if (len < width) {
            if (width > (tmp_buf_ptr - tmp_buf))
                width = (tmp_buf_ptr - tmp_buf);
            if (flags & MINUS_FLAG) {
                memcpy(buffer, tmp_buf_ptr, len);
                memset(buffer + len, (flags & ZERO_PADDING) ? '0' : ' ', width - len);
                len = width;
            } else {
                memset(buffer, (flags & ZERO_PADDING) ? '0' : ' ', width - len);
                memcpy(buffer + width - len, tmp_buf_ptr, len);
                len = width;
            }
        } else {
            memcpy(buffer, tmp_buf_ptr, len);
        }
        xfree(tmp_buf);
        return len;
    } else {
        memcpy(buffer, tmp_buf_ptr, buf_size);
        xfree(tmp_buf);
        return buf_size;
    }
}

#ifndef KERNEL

int snprintf_convert_float(char *buffer, size_t buf_size, double dbl_val, int flags, int width, int precision, char format_char)
{
    char print_buf[160], print_buf_len = 0;
    char format_str[80], *format_str_ptr;

    format_str_ptr = format_str;

    if (width > 155)
        width = 155;
    if (precision <= 0)
        precision = 6;
    if (precision > 120)
        precision = 120;

    /* Construct the formatting string and let system's sprintf
       do the real work. */

    *format_str_ptr++ = '%';

    if (flags & MINUS_FLAG)
        *format_str_ptr++ = '-';
    if (flags & PLUS_FLAG)
        *format_str_ptr++ = '+';
    if (flags & SPACE_FLAG)
        *format_str_ptr++ = ' ';
    if (flags & ZERO_PADDING)
        *format_str_ptr++ = '0';
    if (flags & HASH_FLAG)
        *format_str_ptr++ = '#';

    sprintf(format_str_ptr, "%d.%d", width, precision);
    format_str_ptr += strlen(format_str_ptr);

    if (flags & IS_LONG_DOUBLE)
        *format_str_ptr++ = 'L';
    *format_str_ptr++ = format_char;
    *format_str_ptr++ = '\0';

    sprintf(print_buf, format_str, dbl_val);
    print_buf_len = strlen(print_buf);

    if (print_buf_len > buf_size)
        print_buf_len = buf_size;
    strncpy(buffer, print_buf, print_buf_len);
    return print_buf_len;
}

#endif                          /* KERNEL */

int snprintf(char *str, size_t size, const char *format, ...)
{
    int ret;
    va_list ap;

    va_start(ap, format);
    ret = vsnprintf(str, size, format, ap);
    va_end(ap);

    return ret;
}

int vsnprintf(char *str, size_t size, const char *format, va_list ap)
{
    int status, left = (int) size - 1;
    const char *format_ptr = format;
    int flags, width, precision, i;
    char format_char, *orig_str = str;
    int *int_ptr;
    long int long_val;
    unsigned long int ulong_val;
    char *str_val;

#ifndef KERNEL
    double dbl_val;
#endif                          /* KERNEL */

    flags = 0;
    while (format_ptr < format + strlen(format)) {
        if (*format_ptr == '%') {
            if (format_ptr[1] == '%' && left > 0) {
                *str++ = '%';
                left--;
                format_ptr += 2;
            } else {
                if (left <= 0) {
                    *str = '\0';
                    return size;
                } else {
                    status = snprintf_get_directive(format_ptr, &flags, &width, &precision, &format_char, &ap);
                    if (status == 0) {
                        *str = '\0';
                        return 0;
                    } else {
                        format_ptr += status;
                        /* Print a formatted argument */
                        switch (format_char) {
                        case 'i':
                        case 'd':
                            /* Convert to unsigned long int before
                               actual conversion to string */
                            if (flags & IS_LONG_INT)
                                long_val = va_arg(ap, long int);

                            else
                                long_val = (long int) va_arg(ap, int);

                            if (long_val < 0) {
                                ulong_val = (unsigned long int) -long_val;
                                flags |= IS_NEGATIVE;
                            } else {
                                ulong_val = (unsigned long int) long_val;
                            }
                            status = snprintf_convert_ulong(str, left, 10, "0123456789", ulong_val, flags, width, precision);
                            str += status;
                            left -= status;
                            break;

                        case 'x':
                            if (flags & IS_LONG_INT)
                                ulong_val = va_arg(ap, unsigned long int);

                            else
                                ulong_val = (unsigned long int) va_arg(ap, unsigned int);

                            status = snprintf_convert_ulong(str, left, 16, "0123456789abcdef", ulong_val, flags, width, precision);
                            str += status;
                            left -= status;
                            break;

                        case 'X':
                            if (flags & IS_LONG_INT)
                                ulong_val = va_arg(ap, unsigned long int);

                            else
                                ulong_val = (unsigned long int) va_arg(ap, unsigned int);

                            status = snprintf_convert_ulong(str, left, 16, "0123456789ABCDEF", ulong_val, flags, width, precision);
                            str += status;
                            left -= status;
                            break;

                        case 'o':
                            if (flags & IS_LONG_INT)
                                ulong_val = va_arg(ap, unsigned long int);

                            else
                                ulong_val = (unsigned long int) va_arg(ap, unsigned int);

                            status = snprintf_convert_ulong(str, left, 8, "01234567", ulong_val, flags, width, precision);
                            str += status;
                            left -= status;
                            break;

                        case 'u':
                            if (flags & IS_LONG_INT)
                                ulong_val = va_arg(ap, unsigned long int);

                            else
                                ulong_val = (unsigned long int) va_arg(ap, unsigned int);

                            status = snprintf_convert_ulong(str, left, 10, "0123456789", ulong_val, flags, width, precision);
                            str += status;
                            left -= status;
                            break;

                        case 'p':
                            break;

                        case 'c':
                            if (flags & IS_LONG_INT)
                                ulong_val = va_arg(ap, unsigned long int);

                            else
                                ulong_val = (unsigned long int) va_arg(ap, unsigned int);

                            *str++ = (unsigned char) ulong_val;
                            left--;
                            break;

                        case 's':
                            str_val = va_arg(ap, char *);

                            if (str_val == NULL)
                                str_val = "(null)";

                            if (precision == 0)
                                precision = strlen(str_val);
                            else {
                                if (memchr(str_val, 0, precision) != NULL)
                                    precision = strlen(str_val);
                            }
                            if (precision > left)
                                precision = left;

                            if (width > left)
                                width = left;
                            if (width < precision)
                                width = precision;
                            i = width - precision;

                            if (flags & MINUS_FLAG) {
                                strncpy(str, str_val, precision);
                                memset(str + precision, (flags & ZERO_PADDING) ? '0' : ' ', i);
                            } else {
                                memset(str, (flags & ZERO_PADDING) ? '0' : ' ', i);
                                strncpy(str + i, str_val, precision);
                            }
                            str += width;
                            left -= width;
                            break;

                        case 'n':
                            int_ptr = va_arg(ap, int *);

                            *int_ptr = str - orig_str;
                            break;

#ifndef KERNEL
                        case 'f':
                        case 'e':
                        case 'E':
                        case 'g':
                        case 'G':
                            if (flags & IS_LONG_DOUBLE)
                                dbl_val = (double) va_arg(ap, long double);

                            else
                                dbl_val = va_arg(ap, double);

                            status = snprintf_convert_float(str, left, dbl_val, flags, width, precision, format_char);
                            str += status;
                            left -= status;
                            break;
#endif                          /* KERNEL */

                        default:
                            break;
                        }
                    }
                }
            }
        } else {
            if (left > 0) {
                *str++ = *format_ptr++;
                left--;
            } else {
                *str = '\0';
                return size;
            }
        }
    }
    *str = '\0';
    return size - left - 1;
}
