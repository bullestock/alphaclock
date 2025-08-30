#include "console.h"
#include "defs.h"
#include "hand.h"
#include "hw.h"
#include "nvs.h"
#include "stepper.h"

#include "esp_system.h"
#include "esp_log.h"
#include <esp_console.h>
#include <esp_timer.h>
#include <driver/uart_vfs.h>
#include <driver/uart.h>

#include "linenoise/linenoise.h"
#include "argtable3/argtable3.h"

extern Stepper s_hours, s_minutes, s_seconds;
extern Hand h_hours, h_minutes, h_seconds;

static int reboot(int, char**)
{
    printf("Reboot...\n");
    esp_restart();
    return 0;
}

static int zero(int, char**)
{
    printf("Zeroing\n");

    h_hours.zero();
    h_minutes.zero();
    h_seconds.zero();
    
    return 0;
}

struct
{
    struct arg_int* motor;
    struct arg_int* delay;
    struct arg_int* steps;
    struct arg_end* end;
} motor_args;

static int test_motor(int argc, char** argv)
{
    int nerrors = arg_parse(argc, argv, (void**) &motor_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, motor_args.end, argv[0]);
        return 1;
    }
    const auto motor = motor_args.motor->ival[0];
    if (motor < 0 || motor >= MOTOR_COUNT)
    {
        printf("ERROR: Invalid motor: %d\n", motor);
        return 1;
    }
    const auto delay = motor_args.delay->ival[0];
    const auto steps = motor_args.steps->ival[0];

    Stepper* motors[] = { &s_hours, &s_minutes, &s_seconds };
    
    printf("Stepping motor %d at %d us: %d\n",
           motor, delay, steps);

    motors[motor]->step(steps, delay, true);

    printf("Done\n");

    return 0;
}

struct
{
    struct arg_str* hand;
    struct arg_int* where;
    struct arg_end* end;
} hand_args;

static int hand(int argc, char** argv)
{
    int nerrors = arg_parse(argc, argv, (void**) &hand_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, hand_args.end, argv[0]);
        return 1;
    }
    const auto hand = hand_args.hand->sval[0];
    Hand* h = nullptr;
    if ((hand[0] == 'h') || (hand[0] == 'H'))
        h = &h_hours;
    else if ((hand[0] == 'm') || (hand[0] == 'M'))
        h = &h_minutes;
    else if ((hand[0] == 's') || (hand[0] == 'S'))
        h = &h_seconds;
    else
    {
        printf("ERROR: Invalid hand: %s\n", hand);
        return 1;
    }
    const auto where = hand_args.where->ival[0];

    h->go_to(where);

    printf("Done\n");

    return 0;
}

struct
{
    struct arg_int* motor;
    struct arg_int* reverse;
    struct arg_dbl* steps;
    struct arg_end* end;
} calibrate_args;

static int calibrate(int argc, char** argv)
{
    if (argc <= 1)
    {
        printf("Calibration data:\n");
        for (int i = 0; i < MOTOR_COUNT; ++i)
        {
            const auto& calibration = get_calibration(i);
            printf("%d  %1d  %5.1f\n", i,
                   calibration.reverse,
                   calibration.steps);
        }
        return 0;
    }
    int nerrors = arg_parse(argc, argv, (void**) &calibrate_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, calibrate_args.end, argv[0]);
        return 1;
    }
    const auto motor = calibrate_args.motor->ival[0];
    if (motor < 0 || motor >= MOTOR_COUNT)
    {
        printf("ERROR: Invalid motor: %d\n", motor);
        return 1;
    }
    const auto reverse = calibrate_args.reverse->ival[0];
    const auto steps = calibrate_args.steps->dval[0];

    set_calibration(motor, reverse, steps);

    printf("Done\n");

    return 0;
}

struct
{
    struct arg_str* ssid;
    struct arg_str* password;
    struct arg_end* end;
} add_wifi_credentials_args;

int add_wifi_credentials(int argc, char** argv)
{
    int nerrors = arg_parse(argc, argv, (void**) &add_wifi_credentials_args);
    if (nerrors != 0)
    {
        arg_print_errors(stderr, add_wifi_credentials_args.end, argv[0]);
        return 1;
    }
    const auto ssid = add_wifi_credentials_args.ssid->sval[0];
    const auto password = add_wifi_credentials_args.password->sval[0];
    if (strlen(ssid) < 1)
    {
        printf("ERROR: Invalid SSID value\n");
        return 1;
    }
    add_wifi_credentials(ssid, password);
    printf("OK: Added WiFi credentials %s/%s\n", ssid, password);
    return 0;
}

int list_wifi_creds(int argc, char** argv)
{
    const auto creds = get_wifi_creds();
    for (const auto& c : creds)
    {
        printf("%-20s %s\n", c.first.c_str(),
               c.second.empty() ? "" : "***");
    }
    printf("OK: Listed %d WiFi credentials\n", static_cast<int>(creds.size()));
    return 0;
}

int clear_wifi_credentials(int, char**)
{
    clear_wifi_credentials();
    printf("OK: WiFi credentials cleared\n");
    return 0;
}


void initialize_console()
{
    setvbuf(stdin, NULL, _IONBF, 0);
    uart_vfs_dev_port_set_rx_line_endings(0, ESP_LINE_ENDINGS_CR);
    uart_vfs_dev_port_set_tx_line_endings(0, ESP_LINE_ENDINGS_CRLF);

    uart_config_t uart_config;
    memset(&uart_config, 0, sizeof(uart_config));
    uart_config.baud_rate = CONFIG_ESP_CONSOLE_UART_BAUDRATE;
    uart_config.data_bits = UART_DATA_8_BITS;
    uart_config.parity = UART_PARITY_DISABLE;
    uart_config.stop_bits = UART_STOP_BITS_1;
    uart_config.source_clk = UART_SCLK_REF_TICK;
    ESP_ERROR_CHECK(uart_param_config((uart_port_t) CONFIG_ESP_CONSOLE_UART_NUM, &uart_config));

    ESP_ERROR_CHECK(uart_driver_install((uart_port_t) CONFIG_ESP_CONSOLE_UART_NUM,
                                         256, 0, 0, NULL, 0));

    uart_vfs_dev_use_driver(CONFIG_ESP_CONSOLE_UART_NUM);

    esp_console_config_t console_config;
    memset(&console_config, 0, sizeof(console_config));
    console_config.max_cmdline_args = 8;
    console_config.max_cmdline_length = 256;
#if CONFIG_LOG_COLORS
    console_config.hint_color = atoi(LOG_COLOR_CYAN);
#endif
    ESP_ERROR_CHECK(esp_console_init(&console_config));

    /* Configure linenoise line completion library */
    /* Enable multiline editing. If not set, long commands will scroll within
     * single line.
     */
    linenoiseSetMultiLine(1);

    /* Tell linenoise where to get command completions and hints */
    linenoiseSetCompletionCallback(&esp_console_get_completion);
    linenoiseSetHintsCallback((linenoiseHintsCallback*) &esp_console_get_hint);

    /* Set command history size */
    linenoiseHistorySetMaxLen(100);
}

void run_console()
{
    initialize_console();

    esp_console_register_help_command();

    const esp_console_cmd_t reboot_cmd = {
        .command = "reboot",
        .help = "Reboot",
        .hint = nullptr,
        .func = &reboot,
        .argtable = nullptr,
        .func_w_context = nullptr,
        .context = nullptr
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&reboot_cmd));

    motor_args.motor = arg_int1(NULL, NULL, "<motor>", "Motor (0, 1, 2)");
    motor_args.delay = arg_int1(NULL, NULL, "<delay>", "Delay (us)");
    motor_args.steps = arg_int1(NULL, NULL, "<steps>", "Number of steps");
    motor_args.end = arg_end(2);
    const esp_console_cmd_t test_motor_cmd = {
        .command = "motor",
        .help = "Test motor",
        .hint = nullptr,
        .func = &test_motor,
        .argtable = &motor_args,
        .func_w_context = nullptr,
        .context = nullptr
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&test_motor_cmd));

    hand_args.hand = arg_str1(NULL, NULL, "<hand>", "Hand (h, m, s)");
    hand_args.where = arg_int1(NULL, NULL, "<where>", "Where (0-59)");
    hand_args.end = arg_end(2);
    const esp_console_cmd_t hand_cmd = {
        .command = "hand",
        .help = "Set a hand to a position",
        .hint = nullptr,
        .func = &hand,
        .argtable = &hand_args,
        .func_w_context = nullptr,
        .context = nullptr
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&hand_cmd));

    calibrate_args.motor = arg_int1(NULL, NULL, "<motor>", "Motor (0, 1, 2)");
    calibrate_args.reverse = arg_int1(NULL, NULL, "<reverse>", "Reverse (0, 1)");
    calibrate_args.steps = arg_dbl1(NULL, NULL, "<steps>", "Steps needed for a complete rotation)");
    calibrate_args.end = arg_end(2);
    const esp_console_cmd_t calibrate_cmd = {
        .command = "calibrate",
        .help = "Calibrate motors",
        .hint = nullptr,
        .func = &calibrate,
        .argtable = &calibrate_args,
        .func_w_context = nullptr,
        .context = nullptr
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&calibrate_cmd));

    const esp_console_cmd_t zero_cmd = {
        .command = "zero",
        .help = "Set zero position",
        .hint = nullptr,
        .func = &zero,
        .argtable = nullptr,
        .func_w_context = nullptr,
        .context = nullptr
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&zero_cmd));

    add_wifi_credentials_args.ssid = arg_str1(NULL, NULL, "<ssid>", "SSID");
    add_wifi_credentials_args.password = arg_strn(NULL, NULL, "<password>", 0, 1, "Password");
    add_wifi_credentials_args.end = arg_end(2);
    const esp_console_cmd_t add_wifi_credentials_cmd = {
        .command = "wifi",
        .help = "Add WiFi credentials",
        .hint = nullptr,
        .func = &add_wifi_credentials,
        .argtable = &add_wifi_credentials_args,
        .func_w_context = nullptr,
        .context = nullptr
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&add_wifi_credentials_cmd));

    const esp_console_cmd_t list_wifi_credentials_cmd = {
        .command = "list_wifi",
        .help = "List WiFi credentials",
        .hint = nullptr,
        .func = &list_wifi_creds,
        .argtable = nullptr,
        .func_w_context = nullptr,
        .context = nullptr
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&list_wifi_credentials_cmd));

    const esp_console_cmd_t clear_wifi_credentials_cmd = {
        .command = "clearwifi",
        .help = "Clear WiFi credentials",
        .hint = nullptr,
        .func = &clear_wifi_credentials,
        .argtable = nullptr,
        .func_w_context = nullptr,
        .context = nullptr
    };
    ESP_ERROR_CHECK(esp_console_cmd_register(&clear_wifi_credentials_cmd));

    const char* prompt = LOG_COLOR_I "alphaclock> " LOG_RESET_COLOR;
    int probe_status = linenoiseProbe();
    if (probe_status)
    {
        printf("\n"
               "Your terminal application does not support escape sequences.\n"
               "Line editing and history features are disabled.\n"
               "On Windows, try using Putty instead.\n");
        linenoiseSetDumbMode(1);
#if CONFIG_LOG_COLORS
        /* Since the terminal doesn't support escape sequences,
         * don't use color codes in the prompt.
         */
        prompt = "alphaclock> ";
#endif // CONFIG_LOG_COLORS
    }

    while (true)
    {
        char* line = linenoise(prompt);
        if (!line)
            continue;

        linenoiseHistoryAdd(line);

        int ret;
        esp_err_t err = esp_console_run(line, &ret);
        if (err == ESP_ERR_NOT_FOUND)
            printf("Unrecognized command\n");
        else if (err == ESP_ERR_INVALID_ARG)
            ; // command was empty
        else if (err == ESP_OK && ret != ESP_OK)
            printf("Command returned non-zero error code: 0x%x (%s)\n", ret, esp_err_to_name(err));
        else if (err != ESP_OK)
            printf("Internal error: %s\n", esp_err_to_name(err));

        linenoiseFree(line);
    }
}

// Local Variables:
// compile-command: "(cd ..; idf.py build)"
// End:
