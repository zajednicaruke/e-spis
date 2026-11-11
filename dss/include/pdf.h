#ifndef PDF_H
#define PDF_H
#include <stddef.h>

/* Generiraj PDF nalaz iz report buffera u folder_path.
   Vraća 0 = uspjeh, -1 = greška. */
int pdf_generiraj(const char *folder_path,
                  const char *report_text,
                  size_t report_size);

#endif
