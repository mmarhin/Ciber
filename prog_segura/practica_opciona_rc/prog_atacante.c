#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int
main ()
{
  char *target = "/etc/passwd";
  char *linkname = "/tmp/XYZ";

  unlink (linkname);

  if (symlink (target, linkname) == -1)
    {
      perror ("Error al crear el enlace simbólico");
      return 1;
    }

  printf ("Enlace simbólico creado: %s -> %s\n", linkname, target);
  return 0;
}
