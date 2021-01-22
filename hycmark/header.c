#include <string.h>
#include <ctype.h>
#include "header.h"

/* Read key, lowercase and collapse runs of whitespace. */
int
read_metadata_key (const char *s, char const **es, char buf[], int buflen)
{
  char *t;

  if (!isalnum (*s))
    return 0;
  t = buf;
  do
    if (t < &buf[buflen - 1])
      {
	if (!isspace (*s))
	  *t++ = tolower (*s++);
	else
	  {
	    *t++ = ' ';
	    do
	      s++;
	    while (isspace (*s));
	  }
      }
  while (isalnum (*s) || isspace (*s) || strchr ("_-", *s) != NULL);

  /* remove trailing blanks */
  while (isspace (t[-1]))
    t--;
  *t = '\0';

  if (es != NULL)
    *es = s;
  return t - buf;
}

int
read_metadata_value (const char *s, char const **es, char buf[], int buflen)
{
  char c, *t;

  t = buf;
  do
    {
      /* skip leading space */
      while (isspace (*s) && *s != '\n')
	s++;
      /* read up to newline, ignore \r */
      while ((c = *s++) != '\0')
	{
	  if (c != '\r' && t < &buf[buflen - 1])
	    *t++ = c;
	  if (c == '\n')
	    break;
	}
    }
  while (c != '\0' && *s == ' '); /* line starting with space marks continuation line */

  /* remove trailing blanks */
  while (isspace (t[-1]))
    t--;
  *t = '\0';

  if (es != NULL)
    *es = s;
  return t - buf;
}

