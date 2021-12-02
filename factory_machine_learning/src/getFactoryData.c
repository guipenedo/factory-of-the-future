#include <iomanip.h>
#include <sstream.h>
#include <vector.h>
#include <fstream.h>
#include <unistd.h>

#include<convert_hour.c> //for takedata()
#include<ml_module.c> //for read_Sensor_data_from_file()


void DataArray()
{   
    while (1)
    {
        FILE *hr1, *hr2, *hr3, *temp1, *temp2, *temp3;
        hr1 = fopen("hour1.csv", "w+");
        hr2 = fopen("hour2.csv", "w+");
        hr3 = fopen("hour3.csv", "w+");
        temp1 = fopen("temperature1.csv", "w+");
        temp2 = fopen("tempertature2.csv", "w+");
        temp3 = fopen("temperature3.csv", "w+");
        
        SensorData * data = malloc(100 * sizeof(SensorData));
        int lines = read_sensor_data_from_file(data, 100, factory_client->host_id);
       
        for (int i = 0; i < lines; ++i) {
            //printf("Ts=%ld T=%lf H=%lf P=%lf\n", data[i].time, data[i].temperature, data[i].humidity, data[i].pressure);
            
            char* value = strtok(Ts), ",");
            double Temperature = takedate(value);
            fprintf(hr1,"%ld\n", Temperature);
            fprintf(temp1,"%lf\n", T);
        }
        free(data);

        //repeat for the 2 other factories

        fclose(hr1);
        fclose(hr2);
        fclose(hr3);
        fclose(temp1);
        fclose(temp2);
        fclose(temp3);

        sleep(600);

        remove("hour1.csv");
        remove("hour2.csv");
        remove("hour3.csv");
        remove("temperature1.csv");
        remove("temperature2.csv");
        remove("temperature3.csv");
    }   
}


