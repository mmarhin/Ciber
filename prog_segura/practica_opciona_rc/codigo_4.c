#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
int
main ()
{
  char *fn = "/tmp/XYZ";
  char buffer[60];
  FILE *fp;
  /* get user input */
  scanf ("%50s", buffer);
  if (!access (fn, W_OK))
    {
      fp = fopen (fn, "a+");
      if (!fp)
	{
	  perror ("Ha fallado la apertura");
	  exit (1);
	}
      fwrite ("\n", sizeof (char), 1, fp);
      fwrite (buffer, sizeof (char), strlen (buffer), fp);
      fclose (fp);
    }
  else
    {
      printf ("No se tiene permiso \n");
    }
  return 0;
}
