#include <stdio.h>
#include <string.h>

void llamada_strcpy(/*@in@*/ char *entrada, /*@out@*/ char *salida)
{
	strcpy(entrada, salida);
}



int main ()
{
  char qfoo[9];
  char q0[9];
  int qbar = 0;
  char qbaz;
  printf
    ("Introduce el DNI caracter a caracter y presiona Enter para terminar):\n");
  while ((qbaz = getchar ()) != '\n')
    {
      qfoo[qbar++] = qbaz;
    }
  qfoo[qbar] = '\0';
  llamada_strcpy (q0, qfoo);
  printf ("El DNI introducido es: %s\n", q0);
  return 0;
}
