#include "headers.h"

#define NO_COLOR "\033[0m"
#define RED "\033[0;31m"
#define MAIN_COLOR "\033[1;34m"
#define NOTIF_COLOR "\033[1;36m"
#define TIME_COLOR "\033[0;36m"
char * main_color = MAIN_COLOR;
char * notification_color = NOTIF_COLOR;

#define BATTERY_ALLERT_VALUE 30
int current_battery;

char current_time[6];
char current_path[64];

void update_time() {
    FILE * fp = popen("date +'%I:%M'", "r");
    if (fp == NULL) {
        perror("no date\n");
        return;
    }
    fgets(current_time, sizeof(current_time), fp);
}

bool update_battery_level() {
    FILE * fp = fopen("/sys/class/power_supply/BAT0/capacity", "rt");
    if (fp == NULL) {
        perror("can't find /sys/class/power_supply/BAT0/capacity\n");
        return false;
    }
    fscanf(fp, "%d", & current_battery);
    fclose(fp);
    if (current_battery <= BATTERY_ALLERT_VALUE) 
        return true;

    return false;
}

void update_current_dir_path() {
    FILE * fp = popen("pwd", "r");
    if (fp == NULL) 
        perror("Failed to call pwd with popen");
    
    fgets(current_path, sizeof(current_path), fp);
    current_path[strlen(current_path) - 1] = '\0';
    pclose(fp);
}

void terminal() {
    if (has_exec_failed)
        notification_color = RED;
    else
        notification_color = NOTIF_COLOR;

    printf("%s%s ", MAIN_COLOR, current_path);
    printf("%s%s ", TIME_COLOR, current_time);
    if (update_battery_level()) 
        printf("%s%d ", RED, current_battery);
    
    printf("\n");
    printf("%s> $ %s", notification_color, NO_COLOR);
}