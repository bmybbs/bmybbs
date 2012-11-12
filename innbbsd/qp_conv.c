/* ----------------------------------------------------- */
/* QP code : "0123456789ABCDEF"                          */
/* ----------------------------------------------------- */

static int
qp_code(x)
  register int x;
{
  if (x >= '0' && x <= '9')
    return x - '0';
  if (x >= 'a' && x <= 'f')
    return x - 'a' + 10;
  if (x >= 'A' && x <= 'F')
    return x - 'A' + 10;
  return -1;
}


/* ------------------------------------------------------------------ */
/* BASE64 :                                                           */
/* "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/" */
/* ------------------------------------------------------------------ */

static int
base64_code(x)
  register int x;
{
  if (x >= 'A' && x <= 'Z')
    return x - 'A';
  if (x >= 'a' && x <= 'z')
    return x - 'a' + 26;
  if (x >= '0' && x <= '9')
    return x - '0' + 52;
  if (x == '+')
    return 62;
  if (x == '/')
    return 63;
  return -1;
}

int 
ignorestr(str)
char *str;
{
      char *s;
        
      for(s=str;*s != '\0'; s++) {
          if(!isalnum(*s) &&!ispunct(*s)&&!isspace(*s)) 
             return 1;
      }
      return 0;
}


/* ----------------------------------------------------- */
/* judge & decode QP / BASE64                            */
/* ----------------------------------------------------- */


void
str_decode(dst, src)
  register unsigned char *dst, *src;
{
  register int is_qp, is_base64, is_done;
  register int c1, c2, c3, c4;

  if(ignorestr(src))
  {
     strcpy(dst,src);
     return;
  }
  for (is_done = is_qp = is_base64 = 0; c1 = *src; src++)
  {
    if (c1 == '?' && src[1] == '=')
    {
      src++;
      continue;
    }
    else if (c1 == '\n')        /* chuan: multi line encoding */
    {
      src++;
      is_done = is_qp = is_base64 = 0;
      continue;
    }
    else if (is_qp && c1 == '=')
    {
      c1 = *++src;
      c2 = *++src;
      *dst++ = (qp_code(c1) << 4) | qp_code(c2);
    }
    else if (is_base64 && !is_done)
    {
      while (isspace(c1))
      {
        c1 = *++src;
      }
      if (!c1)
        break;
      do
      {
        c2 = *++src;
      } while (isspace(c2));
      if (!c2)
        break;
      do
      {
        c3 = *++src;
      } while (isspace(c3));
      if (!c3)
        break;
      do
      {
        c4 = *++src;
      } while (isspace(c4));
      if (!c4)
        break;
      if (c1 == '=' || c2 == '=')
      {
        is_done = 1;
        continue;
      }
      c2 = base64_code(c2);
      *dst++ = (base64_code(c1) << 2) | ((c2 & 0x30) >> 4);
      if (c3 == '=')
        is_done = 1;
      else
      {
        c3 = base64_code(c3);
        *dst++ = ((c2 & 0xF) << 4) | ((c3 & 0x3c) >> 2);
        if (c4 == '=')
          is_done = 1;
        else
        {
          *dst++ = ((c3 & 0x03) << 6) | base64_code(c4);
        }
      }
    }
    else if ((c1 == '=') && (src[1] == '?'))
    {
      /* c2 : qmarks, c3 : code_kind */

      c2 = c3 = 0;

      for (;;)
      {
        c1 = *++src;
	if (c1==0) break;
        if (c1 != '?')
        {
          if (c2 == 2)
            c3 = c1 | 0x20;
        }
        else
        {
          if (++c2 >= 3)
            break;
        }
      }

      if (c3 == 'q')
        is_qp = 1;
      else if (c3 == 'b')
        is_base64 = 1;
    }
    else
      *dst++ = c1;
  }
  *dst = '\0';
}

