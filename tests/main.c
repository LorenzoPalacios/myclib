#include <stdio.h>

#include "framework.h"

int main(void) {
  setvbuf(stdout, NULL, _IOFBF, BUFSIZ);
  load_config();
  main_menu_prompt();
  return 0;
}
