#include <stdbool.h>

const char *bool_to_word (bool value)
{
  if (value == true) {
    return "Yes";
  }
  else {
    return "No";
  }
}