ssize_t find_short(const char *s)
{
  int counter = 0;
  int num = 100;
  for (unsigned long i = 0; i < strlen(s); i++) {
    if (s[i] != ' ') {
      counter++;
    }
    else {
      if (counter != 0 && counter < num) {
        num = counter;
      }
      counter = 0;
    }
  }
  if (counter != 0 && counter < num) {
        num = counter;
      }
  return num;