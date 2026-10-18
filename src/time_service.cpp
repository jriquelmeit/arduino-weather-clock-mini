#include "time_service.h"
#include <coredecls.h>

static const char* SPANISH_DAYS[] = {
    "Domingo", "Lunes", "Martes", "Miercoles", "Jueves", "Viernes", "Sabado"
};

static const char* SPANISH_SHORT_DAYS[] = {
    "DOM", "LUN", "MAR", "MIE", "JUE", "VIE", "SAB"
};

static const char* SPANISH_MONTHS[] = {
    "Enero", "Febrero", "Marzo", "Abril", "Mayo", "Junio",
    "Julio", "Agosto", "Septiembre", "Octubre", "Noviembre", "Diciembre"
};

static const char* SPANISH_SHORT_MONTHS[] = {
    "ENE", "FEB", "MAR", "ABR", "MAY", "JUN",
    "JUL", "AGO", "SEP", "OCT", "NOV", "DIC"
};

void initTimeService(const char* tzRule, const char* server1, const char* server2) {
    configTime(tzRule, server1, server2);
}

bool isTimeSynchronized() {
    time_t now = time(nullptr);
    return (now > (time_t)1704067200LL); // Posterior a 01-Ene-2024
}

const char* getSpanishDayName(int wday) {
    if (wday >= 0 && wday < 7) {
        return SPANISH_DAYS[wday];
    }
    return "";
}

const char* getSpanishShortDayName(int wday) {
    if (wday >= 0 && wday < 7) {
        return SPANISH_SHORT_DAYS[wday];
    }
    return "";
}

const char* getSpanishMonthName(int month) {
    if (month >= 1 && month <= 12) {
        return SPANISH_MONTHS[month - 1];
    }
    return "";
}

const char* getSpanishShortMonthName(int month) {
    if (month >= 1 && month <= 12) {
        return SPANISH_SHORT_MONTHS[month - 1];
    }
    return "";
}

bool getCurrentDateTime(DateTimeInfo& outInfo) {
    time_t now = time(nullptr);
    if (now < 100000) {
        return false;
    }

    struct tm timeinfo;
    localtime_r(&now, &timeinfo);

    outInfo.hour = timeinfo.tm_hour;
    outInfo.minute = timeinfo.tm_min;
    outInfo.second = timeinfo.tm_sec;
    outInfo.day = timeinfo.tm_mday;
    outInfo.month = timeinfo.tm_mon + 1;
    outInfo.year = timeinfo.tm_year + 1900;
    outInfo.dayOfWeek = timeinfo.tm_wday;
    outInfo.isDST = (timeinfo.tm_isdst > 0);

    snprintf(outInfo.timeStr, sizeof(outInfo.timeStr), "%02d:%02d:%02d",
             outInfo.hour, outInfo.minute, outInfo.second);

    snprintf(outInfo.shortTimeStr, sizeof(outInfo.shortTimeStr), "%02d:%02d",
             outInfo.hour, outInfo.minute);

    snprintf(outInfo.dayName, sizeof(outInfo.dayName), "%s",
             getSpanishDayName(outInfo.dayOfWeek));

    snprintf(outInfo.shortDayName, sizeof(outInfo.shortDayName), "%s",
             getSpanishShortDayName(outInfo.dayOfWeek));

    snprintf(outInfo.dateStr, sizeof(outInfo.dateStr), "%s, %d de %s",
             getSpanishDayName(outInfo.dayOfWeek),
             outInfo.day,
             getSpanishMonthName(outInfo.month));

    snprintf(outInfo.shortDateStr, sizeof(outInfo.shortDateStr), "%s %02d %s",
             getSpanishShortDayName(outInfo.dayOfWeek),
             outInfo.day,
             getSpanishShortMonthName(outInfo.month));

    return true;
}

void getSpanishDayFromIsoDate(const char* isoDate, char* outBuffer, size_t maxLen) {
    if (!isoDate || strlen(isoDate) < 10) {
        snprintf(outBuffer, maxLen, "--");
        return;
    }

    int y = 0, m = 0, d = 0;
    if (sscanf(isoDate, "%d-%d-%d", &y, &m, &d) == 3) {
        struct tm t = {};
        t.tm_year = y - 1900;
        t.tm_mon = m - 1;
        t.tm_mday = d;
        t.tm_isdst = -1;
        mktime(&t);
        snprintf(outBuffer, maxLen, "%s", getSpanishDayName(t.tm_wday));
    } else {
        snprintf(outBuffer, maxLen, "--");
    }
}

void getSpanishShortDayFromIsoDate(const char* isoDate, char* outBuffer, size_t maxLen) {
    if (!isoDate || strlen(isoDate) < 10) {
        snprintf(outBuffer, maxLen, "--");
        return;
    }

    int y = 0, m = 0, d = 0;
    if (sscanf(isoDate, "%d-%d-%d", &y, &m, &d) == 3) {
        struct tm t = {};
        t.tm_year = y - 1900;
        t.tm_mon = m - 1;
        t.tm_mday = d;
        t.tm_isdst = -1;
        mktime(&t);
        snprintf(outBuffer, maxLen, "%s %02d", getSpanishShortDayName(t.tm_wday), d);
    } else {
        snprintf(outBuffer, maxLen, "--");
    }
}
