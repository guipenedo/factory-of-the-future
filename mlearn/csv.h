#ifndef FACTORY_OF_THE_FUTURE_CSV_H
#define FACTORY_OF_THE_FUTURE_CSV_H

#include <stdbool.h>
#include <gsl/gsl_matrix.h>

void save_matrix_to_csv(const gsl_matrix *Z, const int num_features, const int num_examples, const char file_name[]);
void get_matrix_dims(const char path[], int *num_features, int *num_examples);
void load_matrix_from_csv(const char path[], gsl_matrix *Q, const bool intercept);

#endif