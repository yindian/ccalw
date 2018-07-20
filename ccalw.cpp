#include <webview.h>
#include "polyfill.h"
#include "nav_js.h"

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
#include <iomanip>
#include <list>
#include <iostream>

void suppress_warnings(void)
{
    (void)webview_check_url;
    (void)my_printf;
}

static void post_message(struct webview *w, const char *arg)
{
    auto *v = static_cast<std::list<std::pair<std::string, std::string> > *>(w->userdata);
    char *p = strchr(const_cast<char *>(arg), ':');
    if (p)
    {
        *p = '\0';
        v->push_back(std::make_pair(std::string(arg), std::string(p + 1)));
    }
    else
    {
        v->push_back(std::make_pair(std::string(), std::string(arg)));
    }
}

std::string get_cal_js_by_year(short int year)
{
    sb.str("");
    daysinmonth[1] = IsLeapYear(year) ? 29 : 28;
    vdouble vterms, vmoons, vmonth;
    double lastnew, lastmon, nextnew;
    lunaryear(year, vterms, lastnew, lastmon, vmoons, vmonth, nextnew);
    my_printf("<center>\n");
    my_printf("<table border=\"1\" cellspacing=\"1\" width=\"90%%\">\n");
    for (short int i = 1; i <= 12; i++)
    {
        PrintMonth(year, i, vterms, lastnew, lastmon, vmoons, vmonth, nextnew, 1, false, 'u', false);
    }
    my_printf("</table>\n</center>\n");
    std::ostringstream oss;
    oss << "document.getElementById('app').innerHTML = '";
    oss << std::hex << std::setw(2) << std::setfill('0');
    for (int c = sb.sbumpc(); c != std::stringbuf::traits_type::eof(); c = sb.sbumpc())
    {
        if (iscntrl(c))
        {
            if (c == '\n')
            {
                oss << "\\n";
            }
            else
            {
                oss << "\\x" << c;
            }
        }
        else if (c == '\'')
        {
            oss << "\\'";
        }
        else if (c == '\\')
        {
            oss << "\\\\";
        }
        else
        {
            oss.put(c);
        }
    }
    oss << "';\n";
    oss << "document.title = 'Year " << std::dec << year << "';\n";
    oss << "window.external.invoke('title:' + document.title);\n";
    oss << "document.getElementById('year').innerHTML = '" << year << "年';\n";
    return oss.str();
}

bool validate_year(short int year, struct webview *pw)
{
    if (year < 1645 || year > 7000)
    {
        webview_dialog(pw, WEBVIEW_DIALOG_TYPE_ALERT, WEBVIEW_DIALOG_FLAG_ERROR,
                       "ccal", "Invalid year value: year 1645-7000.",
                       NULL, 0);
        return false;
    }
    return true;
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
    if (validate_year(year, &w))
    {
        std::list<std::pair<std::string, std::string> > mq;
        w.width = 640;
        w.height = 480;
        w.resizable = 1;
        w.debug = 1;
        w.userdata = static_cast<void *>(&mq);
        w.external_invoke_cb = post_message;
        if (webview_init(&w) != 0)
        {
            fprintf(stderr, "webview_init() failed!\n");
            return 1;
        }
        webview_eval(&w, polyfill);
        std::cout << nav_js;
        webview_eval(&w, nav_js);
        webview_eval(&w, get_cal_js_by_year(year).data());
        while (webview_loop(&w, 1) == 0)
        {
            short int skip_delta = 0;
            while (!mq.empty())
            {
                const auto msg = mq.front();
                mq.pop_front();
                std::cout << "[msg] " << msg.first << " : " << msg.second << std::endl;
                if (msg.first == "title")
                {
                    webview_set_title(&w, msg.second.data());
                }
                else if (msg.first == "year")
                {
                    short int n = std::stoi(msg.second);
                    if (validate_year(n, &w))
                    {
                        year = n;
                        webview_eval(&w, get_cal_js_by_year(year).data());
                    }
                }
                else if (msg.first == "prev" || msg.first == "next")
                {
                    short int delta = msg.first == "prev" ? -1 : 1;
                    if (delta == skip_delta)
                    {
                    }
                    else if (validate_year(year + delta, &w))
                    {
                        year += delta;
                        webview_eval(&w, get_cal_js_by_year(year).data());
                    }
                    else
                    {
                        webview_eval(&w, "stop()");
                        skip_delta = delta;
                    }
                }
            }
        }
    }
    webview_exit(&w);
    return 0;
}
