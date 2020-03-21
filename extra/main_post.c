
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>
#include <string.h>
#include "sps30_i2c_lib.h"
#include <curl/curl.h>

#define WAIT 600 //待ち時間

int main(int argc, char *argv[]){
    printf("Starting\n");
    if (start(1,0x69,0)) {
        printf("Start ok\n");
    } else {
        printf("Start not ok\n");
    }

    if (start_measure()) {
        printf("Measure start ok\n");
    } else {
        printf("Measure start not ok\n");
    }

    
    FILE *file;
    
    file = fopen("log.csv", "r+");
    if (file == NULL) {
        printf("No log file found, creating new one\n");
        file = fopen("log.csv", "w+");
    }
    fprintf(file, " TIME, Mass Concentration PM1.0, Mass Concentration PM2.5, Mass Concentration PM4.0 , Mass Concentration PM10, Number Concentration PM0.5, Number Concentration PM1.0, Number Concentration PM2.5, Number Concentration PM4.0, Number Concentration PM10, Particle Size\n");
    fclose(file); 

    char data[60];
    printf("Press CTR C to stop\n");

    while(1) {
        printf("Logging..\n");
        
        file = fopen("log.csv", "a");
        if (file == NULL) {
            perror("Error opening file");
        }

        //時間取得
        time_t rawtime;
        struct tm * timeinfo;
        time (&rawtime);
        timeinfo = localtime (&rawtime);
        char *curTime = asctime(timeinfo);
        if (curTime[strlen(curTime)-1] == '\n') curTime[strlen(curTime)-1] = '\0';
        fprintf(file, " %s,", curTime);

        //データ読み込み
        if (read_value(data)) {
            uint32_t result[10];
            uint8_t j = 0;
            uint8_t mismatch = 0;

            //チェックサム確認
            for (int i=0; i<sizeof(data); i+=3) {
                int temp_check;
                char temp[2];
                temp[0] = data[i];
                temp[1] = data[i+1];
                temp_check = CalcCrc(temp);
                if (temp_check != data[i+2]) {
                    printf("Mismatch\n");
                    mismatch = 1;
                } 
            }

            if (!mismatch) {
                //フロート変換、書き込み
                for (int i=0; i<60; i+=6) {
                    result[j] = (data[i] << 24) | (data[i+2] << 16) | (data[i+3] << 8) | (data[i+4]);
                    float f;
                    f = *((float*)&result[j]);
                    fprintf(file, " %.8f", f);
                    if (60 > i+6) {
                        fprintf(file, ", ");
                    }
                    j++;
                }
                sendData(result);
            } else {
                fprintf(file, "Mismatch data");
            }

        } else {
            printf("Read value not ok\n");
            fprintf(file, "Read value failed");
        }


        
        fprintf(file, "\n");
        fclose(file); 

        //待つ
        sleep(WAIT);
    }

    // if (stop_measure()) {
    //     printf("Measure stop ok\n");
    // } else {
    //     printf("Measure stop not ok\n");
    // }

    // sleep(2);

    // if (end()) {
    //     printf("End ok\n");
    // } else {
    //     printf("End not ok\n");
    // }
}

int sendData(uint32_t* buf) {
    char text[800] = "";
    char e[] = "=";
    char a[] = "&";
    char n[10][7] = {
                    "MCPM1",
                    "MCPM25",
                    "MCPM4",
                    "MCPM10",
                    "NCPM05",
                    "NCPM1",
                    "NCPM25",
                    "NCPM4",
                    "NCPM10",
                    "PSize"
                    };
    for (int k=0; k<10; k++) {
        strncat(text,&n[k],7);
        strncat(text,&e,1);
        char temp[9];
        sprintf(temp, "%X", buf[k]);
        strncat(text,&temp,9);
        if (k < 9) {
            strncat(text,&a,1);
        }
    }

    CURL *curl;
    CURLcode res;

    /* In windows, this will init the winsock stuff */ 
    curl_global_init(CURL_GLOBAL_ALL);

    /* get a curl handle */ 
    curl = curl_easy_init();
    if(curl) {
    /* First set the URL that is about to receive our POST. This URL can
        just as well be a https:// URL if that is what should receive the
        data. */ 
    curl_easy_setopt(curl, CURLOPT_URL, "192.168.4.108:3000/");
    /* Now specify the POST data */ 
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, text);

    /* Perform the request, res will get the return code */ 
    res = curl_easy_perform(curl);
    /* Check for errors */ 
    if(res != CURLE_OK)
        fprintf(stderr, "curl_easy_perform() failed: %s\n",
                curl_easy_strerror(res));

    /* always cleanup */ 
    curl_easy_cleanup(curl);
    }
    curl_global_cleanup();
}