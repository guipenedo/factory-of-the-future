#include <stdbool.h>
#include <gsl/gsl_matrix.h>
#include "csv.h"
#include "predict_temp.h"


double predict_temp(const char * beta_file_name, int hour, int minute, int seconds)
{
    double temperature;
    double date = hour + minute / 60.0 + seconds / 3600.0;

    gsl_matrix *beta = gsl_matrix_alloc(2, 1);
    load_matrix_from_csv(beta_file_name, beta, false);

    temperature = gsl_matrix_get(beta, 0, 0) + date * gsl_matrix_get(beta, 1, 0);

    return temperature;
}