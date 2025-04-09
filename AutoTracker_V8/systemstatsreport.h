#ifndef SYSTEMSTATSREPORT_H
#define SYSTEMSTATSREPORT_H

#include "app_includes.h"
#include "autotracker_types.h"

#ifdef JETSON_HEADLESS
#define CPU_TEMP_FILE "/sys/devices/virtual/thermal/thermal_zone0/temp"
#else
//#define CPU_TEMP_FILE "/sys/class/hwmon/hwmon4/temp1_input"
#endif

#define MEMORY_FILE "/proc/meminfo"

class SystemStatsReport : public QObject
{
    Q_OBJECT
public:
    explicit SystemStatsReport(QObject *parent = nullptr);

    void dataToCpuTimes(QByteArrayList &cpu_data, CpuTimes &curr_cpu_time);
    double calculateCpuUsage(CpuTimes &_prev_cpu_time, CpuTimes &_curr_cpu_time);
    void readProcFile(QString filename, QByteArrayList &fileData);

    QString CPU_TEMP_FILE = "/sys/class/hwmon/hwmon3/temp1_input";
    //QString CPU_TEMP_FILE = "/sys/class/hwmon/hwmon4/temp1_input";
    QString memory_file_path = MEMORY_FILE;
    QByteArray getCpuTemperature();
    QTimer *eTimer = nullptr;
    CpuTimes prev_cpu_time;
    bool firstRun = true;

    double cpuUsage = 0.0f;
    double avg_cpuUsage = 0.05f;
    double avg_cpuTemp = 40.0f;
    double total_ram = 8.0f, free_ram = 6.0f, ram_usage = ((total_ram - free_ram) / total_ram)*100.0f;

    unsigned int gpu_temp = 0;
    float gpu_usage = 0.0f, gpu_memory = 0.0f;

#ifndef JETSON_HEADLESS
#ifdef USE_GPU
        QVector<nvmlDevice_t> nvmlDevices;
#endif
#endif

public slots:
    void init();
    void start(int interval);
    void readStats();

signals:
    void statsReady(QString stats);


};

#endif // SYSTEMSTATSREPORT_H
