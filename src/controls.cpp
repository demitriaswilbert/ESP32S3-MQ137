#include "controls.h"

static bool enable_backspace_clear = true;
static bool enable_enter = false;

String rx_str = "";

static std::vector<command_t> commands = {
    {"KB bkspc", 0,
     [](String in_str)
     {
         enable_backspace_clear = !enable_backspace_clear;
         log_cdc(enable_backspace_clear ? "Backspace clears RX buffer"
                                        : "Backspace is normal key");
     }},
    {"KB enter", 0,
     [](String in_str)
     {
         enable_enter = !enable_enter;
         log_cdc(enable_enter ? "Enter is also a terminating key"
                              : "Enter is normal key");
     }},
    {"KB toggle", 0, [](String in_str)
     {
         if (rgb_get_state())
            rgb_stop_task();
         else rgb_init_task(2);

         log_cdc("RGB %s started", rgb_get_state()? "is" : "not");
     }},
    {"KB stat", 0, [](String in_str)
     {
         const char *flashmode = "PLACEHOLDER";
         switch (ESP.getFlashChipMode())
         {
         case FM_QIO:
             flashmode = "FM_QIO";
             break;
         case FM_QOUT:
             flashmode = "FM_QOUT";
             break;
         case FM_DIO:
             flashmode = "FM_DIO";
             break;
         case FM_DOUT:
             flashmode = "FM_DOUT";
             break;
         case FM_FAST_READ:
             flashmode = "FM_FAST_READ";
             break;
         case FM_SLOW_READ:
             flashmode = "FM_SLOW_READ";
             break;
         case FM_UNKNOWN:
             flashmode = "FM_UNKNOWN";
             break;
         default:
             flashmode = "FM_ERROR";
         };
         const uint32_t flashspeed = ESP.getFlashChipSpeed();
         log_cdc("flash_mode: %s, flash_speed: %lu", flashmode, flashspeed);
         log_cdc("flash_size: %lu, free heap: %lu, free psram: %lu", ESP.getFlashChipSize(), ESP.getFreeHeap(), ESP.getFreePsram());
     }},
    {"KB commands", 0, [](String in_str)
     {
         size_t i = 0;
         for (auto &cmd : commands)
         {
             log_cdc("%3d. param length: %3d, Prefix: \"%s\"", i++,
                     cmd.param_len, cmd.prefix.c_str());
         }
     }},

};

int process_command(String &in, command_t *ctl)
{
    if (!ctl->param_len)
    { // if the command structure has no parameter
        if (ctl->prefix == in)
        { // match prefix, execute the function
            ctl->func("");
            return 1;
        }
    }
    else // if the command structure has parameter(s)
    {
        // get prefix string and param length
        const size_t param_len = ctl->param_len;

        // reject if input string is not longer than prefix
        if (in.length() <= ctl->prefix.length())
            return 0;

        // reject if parameter length is longer than accepted param length
        if (in.length() > (ctl->prefix.length() + param_len))
            return 0;

        // reject if input does not start with this command prefix
        if (in.startsWith(ctl->prefix) == 0)
            return 0;

        // send to control function
        String param_str = in.substring(ctl->prefix.length());
        ctl->func(param_str);

        // success
        return 1;
    }
    // does not match
    return 0;
}

void process_char(Stream &in, char c)
{

    /** @brief Clear received string */
    if (c == '\b' && enable_backspace_clear)
    {
        log_cdc("Cleared: %lu\n", rx_str.length());
        rx_str = String();
        return;
    }

    /**
     * @brief received Escape char (\033)
     *
     * start processing received string
     * when received Escape char
     *
     */
    if (isEnd(enable_enter, c))
    {
        if (rx_str.length() == 0)
            return;
        for (auto &i : commands)
        {
            if (process_command(rx_str, &i))
            {
                rx_str = String();
                return;
            }
        }
        rx_str = String();
    }
    else
    {
        rx_str += (char)c;
    }
}

void process_string(Stream &in, const char *buf)
{
    const char *p = buf;
    while (*p)
    {
        process_char(in, *p++);
    }
}