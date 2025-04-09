#include "systemstatsreport.h"

using namespace std;

SystemStatsReport::SystemStatsReport(QObject *parent) : QObject(parent)
{

}

void SystemStatsReport::init()
{
    cout << "Init System Stats: " << QThread::currentThreadId() << endl;
    eTimer = new QTimer;
    connect(eTimer, &QTimer::timeout, this, &SystemStatsReport::readStats);

    //if(!QFile().exists(CPU_TEMP_FILE)) { CPU_TEMP_FILE = "/sys/class/hwmon/hwmon3/temp1_input"; }

#ifdef USE_GPU
    nvmlReturn_t result;
    result = nvmlInit();
    if (result != NVML_SUCCESS){ cout << "NVML init - FAILED" << endl; return; }

    unsigned int device_count;
    result = nvmlDeviceGetCount(&device_count);
    if (result != NVML_SUCCESS){ cout << "NVML Device Count - FAILED" << endl; return; }

    cout << "nvmlDevice Count: " << device_count << endl;

    for(unsigned int i = 0; i < device_count; i++)
    {
        nvmlDevice_t device;
        nvmlDeviceGetHandleByIndex(i, &device);
        nvmlDevices.append(device);
    }
#endif

    start(200);
}

void SystemStatsReport::start(int interval)
{
    if(interval < 0)
        eTimer->stop();
    eTimer->start(interval);
}

void SystemStatsReport::dataToCpuTimes(QByteArrayList &cpu_data, CpuTimes &curr_cpu_time)
{
    if(cpu_data.size() < 11)
        return;
    curr_cpu_time.user = cpu_data[1].toULongLong();
    curr_cpu_time.nice = cpu_data[2].toULongLong();
    curr_cpu_time.system = cpu_data[3].toULongLong();
    curr_cpu_time.idle = cpu_data[4].toULongLong();
    curr_cpu_time.iowait = cpu_data[5].toULongLong();
    curr_cpu_time.irq = cpu_data[6].toULongLong();
    curr_cpu_time.softirq = cpu_data[7].toULongLong();
    curr_cpu_time.steal = cpu_data[8].toULongLong();
    curr_cpu_time.guest = cpu_data[9].toULongLong();
    curr_cpu_time.guest_nice = cpu_data[10].toULongLong();
}

double SystemStatsReport::calculateCpuUsage(CpuTimes &_prev_cpu_time, CpuTimes &_curr_cpu_time)
{
    unsigned long long prevIdle = _prev_cpu_time.idle + _prev_cpu_time.iowait;
    unsigned long long currIdle = _curr_cpu_time.idle + _curr_cpu_time.iowait;

    unsigned long long prevNonIdle = _prev_cpu_time.user + _prev_cpu_time.nice + _prev_cpu_time.system + _prev_cpu_time.irq + _prev_cpu_time.steal;
    unsigned long long currNonIdle = _curr_cpu_time.user + _curr_cpu_time.nice + _curr_cpu_time.system + _curr_cpu_time.irq + _curr_cpu_time.steal;

    unsigned long long prevTotal = prevIdle + prevNonIdle;
    unsigned long long currTotal = currIdle + currNonIdle;

    long long totald = currTotal - prevTotal;
    long long idled = currIdle - prevIdle;

    cpuUsage = ((double)totald - (double)idled) / (double)totald;

    avg_cpuUsage = (avg_cpuUsage*0.9f) + (cpuUsage*0.1f);

    return avg_cpuUsage;
}

void SystemStatsReport::readProcFile(QString filename, QByteArrayList &fileData)
{
    QString filePath = QString("/proc/" + filename);
    QFile procFile(filePath);
    fileData.clear();

    if(procFile.open(QIODevice::ReadOnly))
    {
        QByteArray data = procFile.readAll();
        fileData = data.split('\n');
        procFile.close();
    }
}

QByteArray SystemStatsReport::getCpuTemperature()
{
    QByteArray tempData;
    QFile tempFile(CPU_TEMP_FILE);
    if(tempFile.open(QIODevice::ReadOnly))
    {
        tempData = tempFile.readAll();
        tempData.resize(tempData.size() - 1);
        tempFile.close();
    }
    double cpuTemp = (double)tempData.toInt() / 1000.0;
    avg_cpuTemp = (avg_cpuTemp*0.9f) + (cpuTemp*0.1f);
    tempData = QByteArray::number(avg_cpuTemp, 'f', 0);
    return tempData;
}

void SystemStatsReport::readStats()
{
    QString stats;
    QByteArrayList fileData;
    readProcFile("stat", fileData);

    if(fileData.isEmpty())
        return;

    QByteArrayList cpuStats = fileData[0].split(' ');
    CpuTimes curr_cpu_time;
    dataToCpuTimes(cpuStats, curr_cpu_time);

    if(firstRun){ prev_cpu_time = curr_cpu_time; firstRun = false; return; }

    double cpuUsage = (calculateCpuUsage(prev_cpu_time, curr_cpu_time)) * 100.0f;
    prev_cpu_time = curr_cpu_time;
    stats = QString("CPU: " + QString::number(cpuUsage, 'f', 1) + "% | T: " + getCpuTemperature() + " | ");


    QByteArrayList memoryData;
    readProcFile("meminfo", memoryData);
    if(memoryData.size() > 3)
    {
        QByteArray lineData = memoryData[0].simplified().split(':').last();
        total_ram = lineData.left(lineData.size()-2).toDouble() / 1000000.0f;

        lineData = memoryData[2].simplified().split(':').last();
        free_ram = lineData.left(lineData.size()-2).toDouble() / 1000000.0f;

        ram_usage = ((total_ram - free_ram) / total_ram) * 100.0f;
    }

#ifdef USE_GPU
    for(int i = 0; i < nvmlDevices.size(); i++)
    {
        char device_name[NVML_DEVICE_NAME_BUFFER_SIZE];
        nvmlDeviceGetName(nvmlDevices[i], device_name, NVML_DEVICE_NAME_BUFFER_SIZE);
        QString gpuName(device_name);
        gpuName.remove(0, 15);

        nvmlDeviceGetTemperature(nvmlDevices[i], NVML_TEMPERATURE_GPU, &gpu_temp);

        nvmlUtilization_st device_utilization;
        nvmlDeviceGetUtilizationRates(nvmlDevices[i], &device_utilization);

        gpu_usage = device_utilization.gpu;
        gpu_memory = device_utilization.memory;

        stats.append(QString(gpuName + " : " + QString::number(gpu_usage, 'f', 0) + "% | Mem: " + QString::number(gpu_memory, 'f', 1) + "% | Temp: " + QString::number(gpu_temp, 'f', 0)));
    }
#endif
    emit statsReady(stats);
}
