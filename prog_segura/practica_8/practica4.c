#include <stdio.h>
#include <stdlib.h>
#include <time.h>
void q0 (/*@null@*/ /*@shared@*/int *q1, int q2)
{
  int q3;
  for (q3 = 0; q3 <= q2; q3++)
    {
      printf ("%d", q1[q3]);
    }
    printf ("\n");
} 
int *q4 (int q2)
{
  int *q1 = (int *) malloc (q2 * sizeof (int));
  int q3;  
  for (q3 = 0; q3 < q2; q3++)
    {
      q1[q3] = rand () % 100;
    } return q1;
}

int main ()
{
  srand (time (NULL));
  int q2;
  printf ("Ingrese el tamaño del Array:");
  scanf ("%d", &q2);
  int *qfoo = q4 (q2);
  q0 (qfoo, q2);
  free (qfoo);
  free (qfoo);
  return 0;
}
