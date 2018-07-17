#include <webview.h>
#include "polyfill.h"

#include <sstream>
#include <stdarg.h>
static std::stringbuf sb;
static int my_printf(const char *format, ...)
{
    va_list arg;
    char buf[1024] = {};
    int ret;
    va_start (arg, format);
    ret = vsnprintf (buf, sizeof(buf), format, arg);
    va_end (arg);
    if (ret > 0)
    {
        sb.sputn(buf, ret);
    }
    else
    {
        fprintf(stderr, "printf %s failed!\n", format);
    }
    return ret;
}
#if 1
#define main sub
#define printf my_printf
#include "ccal/ccal.cpp"
#undef main
#undef printf
#endif

#include <ctype.h>

void suppress_warnings(void)
{
    (void)webview_check_url;
    (void)my_printf;
}

int main(int argc, char *argv[])
{
    time_t now = time(NULL);
    struct tm *tmnow = localtime(&now);
    short int year;

    year = (short int) (tmnow->tm_year + 1900);
    if (argc > 1)
    {
        year = atoi(argv[1]);
    }

    struct webview w = {};
    if (year < 1645 || year > 7000)
    {
        webview_dialog(&w, WEBVIEW_DIALOG_TYPE_ALERT, WEBVIEW_DIALOG_FLAG_ERROR,
                       "ccal", "Invalid year value: year 1645-7000.",
                       NULL, 0);
    }
    else
    {
        if (IsLeapYear(year))
        {
            daysinmonth[1] = 29;
        }
        vdouble vterms, vmoons, vmonth;
        double lastnew, lastmon, nextnew;
        lunaryear(year, vterms, lastnew, lastmon, vmoons, vmonth, nextnew);
        char titlestr[20];
        sprintf(titlestr, "Year %d", year);
        my_printf("<html>\n<head>\n<meta http-equiv=\"Content-Type\" ");
        my_printf("content=\"text/html; charset=utf-8\">\n");
        my_printf("<meta name=\"GENERATOR\" content=\"ccal-%s by Zhuo Meng, http://thunder.cwru.edu/ccal/\">\n", versionstr);
        my_printf("<title>Chinese Calendar for %s / %d%s</title>\n", titlestr, year, U8miscchar[16]);
        my_printf("</head>\n<body>\n<center>\n");
        my_printf("<table border=\"1\" cellspacing=\"1\" width=\"90%%\">\n");
        for (short int i = 1; i <= 12; i++)
        {
            PrintMonth(year, i, vterms, lastnew, lastmon, vmoons, vmonth, nextnew, 1, false, 'u', false);
        }
        my_printf("</table>\n</center>\n</body>\n</html>\n");
        std::stringbuf sb2;
        const char *data_url_prefix = "data:text/html,";
        sb2.sputn(data_url_prefix, strlen(data_url_prefix));
        for (int c = sb.sbumpc(); c != std::stringbuf::traits_type::eof(); c = sb.sbumpc())
        {
            if (isalnum(c))
            {
                sb2.sputc(c);
            }
            else
            {
                char buf[4];
                snprintf(buf, sizeof(buf), "%%%02X", c);
                sb2.sputn(buf, 3);
            }
        }
        std::string html = sb2.str();
        w.width = 640;
        w.height = 480;
        w.resizable = 1;
        w.debug = 1;
        w.title = titlestr;
        w.url = html.data();
        if (webview_init(&w) != 0)
        {
            fprintf(stderr, "webview_init() failed!\n");
            return 1;
        }
        webview_eval(&w, polyfill);
        while (webview_loop(&w, 1) == 0)
        {
        }
    }
    webview_exit(&w);
    return 0;
}
