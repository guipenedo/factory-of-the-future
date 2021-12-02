#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_linalg.h>
#include "csv.h"
#include "predict_temp.h"
#include "data_files.h"

#define DIM_SIZE 40


double predict_temp(const char * beta_file_name, const char * x_predict_file_name, int hour, int minute, int seconds)
{
    double temperature = 0.0;
    double date = hour + minute / 60.0 + seconds / 3600.0;
    printf("\n%s", beta_file_name);

    int num_features = 1, num_examples = 1;

    gsl_matrix *X = gsl_matrix_alloc(num_examples, num_features);

    gsl_matrix_set(X, 0, 0, date);

    gsl_matrix *beta = gsl_matrix_alloc(num_features, 1);
    load_matrix_from_csv(beta_file_name, beta, false);
    
    gsl_matrix *y_hat = gsl_matrix_alloc(num_examples, 1);
    gsl_blas_dgemm(CblasNoTrans, CblasNoTrans, 1.0, X, beta, 0.0, y_hat);

    temperature = gsl_matrix_get(y_hat, 0, 0);

	printf("Prediction value = %f", temperature);
    return temperature;
}
//
//int main(int argc, char *argv[]) {
//	float temperature=0.0;
//    if (argc < 3) {
//            printf("wrong value");
//        exit(EXIT_FAILURE);
//    }
//
//    if (strcmp(argv[1], "predict") == 0) {
//	char *factory_id;
//	char *time_of_prediction;
//	factory_id=argv[2];
//	time_of_prediction=argv[3];
//	temperature = predict_temp(factory_id, time_of_prediction);
//    printf("TEMPERATURE final %f", temperature);
//    }
//
//    return 0;
//}
//
//
