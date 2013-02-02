/* title2cl.c

   A small utility to generate a 'character list' used by fntgen
   from titles file.

   Copyright (c) 2006, Wei Mingzhi. All rights reserved.

   This program is free software; the author gives unlimited
   permission to copy, distribute and modify it.
*/

#include <stdio.h>
#include <malloc.h>

void
trim (char *str)
{
  int pos = 0;
  char *dest = str;

  while (str[pos] <= ' ' && str[pos] > 0)
    pos++;

  while (str[pos])
    {
      *(dest++) = str[pos];
      pos++;
    }

  *(dest--) = '\0';

  while (dest >= str && *dest <= ' ' && *dest > 0)
    *(dest--) = '\0';
}

int
alreadyhave (char *buffer, char *end, char *ch)
{
  while (buffer < end)
    {
      if (*buffer == *ch)
        {
          if (*ch & 0x80)
            {
              if ((*ch & 0xE0) == 0xC0)
                {
                  if (*(buffer + 1) == *(ch + 1))
                    {
                      return 2;
                    }
                }
              else if ((*ch & 0xF0) == 0xE0)
                {
                  if (*(buffer + 1) == *(ch + 1)
                      && *(buffer + 2) == *(ch + 2))
                    {
                      return 3;
                    }
                }
            }
          else
            {
              return 1;
            }
        }

      buffer++;
    }

  return 0;
}

int
main (int argc, char *argv[])
{
  FILE *fp;
  int count = 0, flag = 0;
  char *buffer, *p, *p1, buf[256];

  if (argc != 3)
    {
      printf ("Usage: title2cl [titles.txt] [charlist.txt]\n");
      return 1;
    }

  fp = fopen (argv[1], "r");
  if (fp == NULL)
    {
      printf ("Cannot open file: %s\n", argv[1]);
      return 1;
    }

  buffer = (char *) malloc (sizeof (char) * 65536);
  if (buffer == NULL)
    {
      printf ("Memory allocation Error!\n");
      return 1;
    }

  p = buffer;

  while (fgets (buf, 256, fp) != NULL)
    {
      trim (buf);
      if (strcmp (buf, "{") == 0)
        {
          flag = 1;
        }
      else if (strcmp (buf, "}") == 0)
        {
          flag = 0;
        }
      else if (flag)
        {
          p1 = buf;
          while (*p1)
            {
              int t = alreadyhave (buffer, p, p1);
              if (t > 0)
                {
                  p1 += t;
                  continue;
                }
              if ((*p1 & 0x80) == 0)
                {
                  /* this is a standard ASCII character */
                  p1++;
                  continue;
                }
              else if ((*p1 & 0xE0) == 0xC0)
                {
                  *p++ = *p1++;
                  *p++ = *p1++;
                  *p++ = '\n';
                  count++;
                }
              else if ((*p1 & 0xF0) == 0xE0)
                {
                  *p++ = *p1++;
                  *p++ = *p1++;
                  *p++ = *p1++;
                  *p++ = '\n';
                  count++;
                }
            }
        }
    }

  fclose (fp);
  *p = '\0';

  fp = fopen (argv[2], "w");
  if (fp == NULL)
    {
      free (buffer);
      printf ("cannot write to file %s\n", argv[2]);
      return 1;
    }

  fprintf(fp, "%d\n", count + 94);

  /* always include standard ASCII characters */
  fputs ("ABCDEFGHIJKLMNOPQRSTUVWXYZ\n", fp);
  fputs ("abcdefghijklmnopqrstuvwxyz\n", fp);
  fputs ("`1234567890-=\\\n", fp);
  fputs ("~!@#$%^&*()_+|\n", fp);
  fputs ("[]{};\':\",./<>?\n", fp);

  fputs (buffer, fp);
  free (buffer);
  fclose (fp);
}

